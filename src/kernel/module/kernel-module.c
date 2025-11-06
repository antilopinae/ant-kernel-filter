#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <ant-kernel/headers.h>
#include <ant-kernel/utils.h>
#include <ant-kernel/greeter.h>

MODULE_LICENSE(ANT_KERNEL_MODULE_LICENSE);
MODULE_AUTHOR(ANT_KERNEL_MODULE_AUTHOR);
MODULE_DESCRIPTION(ANT_KERNEL_MODULE_DESCRIPTION);

static int major_number;
static struct cdev mmap_cdev;
static mmap_buffer_t *mmap_buffer = NULL;

static int get_next_write_pos(void) {
    int current_idx;
    int next_idx;

    if (atomic_read(&mmap_buffer->num_messages) >= (ANT_MMAP_BUF_SIZE / sizeof(mmap_message_t))) {
        printk(KERN_WARNING "%s: mmap buffer full, dropping message.\n", ANT_KERNEL_MODULE_NAME);
        return -1;
    }

    current_idx = atomic_fetch_add(1, &mmap_buffer->write_idx);
    next_idx = current_idx % (ANT_MMAP_BUF_SIZE / sizeof(mmap_message_t));
    return next_idx;
}

static void send_mmap_message(const char *msg) {
    int idx;
    size_t msg_len;

    if (!mmap_buffer) {
        printk(KERN_ERR "%s: mmap_buffer not initialized!\n", ANT_KERNEL_MODULE_NAME);
        return;
    }

    idx = get_next_write_pos();
    if (idx == -1) {
        return;
    }

    msg_len = strlen(msg);
    if (msg_len >= ANT_MAX_MSG_LEN) {
        msg_len = ANT_MAX_MSG_LEN - 1;
    }

    atomic_set(&mmap_buffer->messages[idx].ready, 0);

    memcpy(mmap_buffer->messages[idx].data, msg, msg_len);
    mmap_buffer->messages[idx].data[msg_len] = '\0';
    mmap_buffer->messages[idx].len = msg_len;

    atomic_inc(&mmap_buffer->num_messages);
    atomic_set(&mmap_buffer->messages[idx].ready, 1);

    smp_wmb();
}

static int kernel_module_open(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "%s: Device opened.\n", ANT_KERNEL_MODULE_NAME);
    return 0;
}

static int kernel_module_release(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "%s: Device closed.\n", ANT_KERNEL_MODULE_NAME);
    return 0;
}

static int kernel_module_mmap(struct file *filp, struct vm_area_struct *vma) {
    int ret;
    unsigned long pfn;

    if ((vma->vm_end - vma->vm_start) > ANT_MMAP_BUF_SIZE) {
        printk(KERN_ERR "%s: mmap region too large.\n", ANT_KERNEL_MODULE_NAME);
        return -EINVAL;
    }

    pfn = virt_to_phys(mmap_buffer) >> PAGE_SHIFT;
    vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

    ret = remap_pfn_range(vma, vma->vm_start, pfn,
                          vma->vm_end - vma->vm_start,
                          vma->vm_page_prot);

    if (ret < 0) {
        printk(KERN_ERR "%s: Failed to mmap: %d\n", ANT_KERNEL_MODULE_NAME, ret);
        return ret;
    }

    printk(KERN_INFO "%s: Mapped buffer to user space at 0x%lx, size %lu bytes.\n",
           ANT_KERNEL_MODULE_NAME, vma->vm_start, vma->vm_end - vma->vm_start);

    return 0;
}

static const struct file_operations kernel_module_fops = {
    .owner = THIS_MODULE,
    .open = kernel_module_open,
    .release = kernel_module_release,
    .mmap = kernel_module_mmap,
};

static int __init kernel_module_init(void) {
    int ret;
    dev_t dev_num;

    ret = alloc_chrdev_region(&dev_num, 0, 1, ANT_KERNEL_MODULE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "%s: Failed to allocate char device region.\n", ANT_KERNEL_MODULE_NAME);
        return ret;
    }

    major_number = MAJOR(dev_num);
    printk(KERN_INFO "%s: Allocated char device with major %d.\n", ANT_KERNEL_MODULE_NAME, major_number);

    cdev_init(&mmap_cdev, &kernel_module_fops);
    mmap_cdev.owner = THIS_MODULE;

    ret = cdev_add(&mmap_cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "%s: Failed to add cdev.\n", ANT_KERNEL_MODULE_NAME);
        return ret;
    }

    mmap_buffer = kmalloc_array(1, sizeof(mmap_buffer_t) + ANT_MMAP_BUF_SIZE, GFP_KERNEL);
    if (!mmap_buffer) {
        cdev_del(&mmap_cdev);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ERR "%s: Failed to allocate mmap buffer.\n", ANT_KERNEL_MODULE_NAME);
        return -ENOMEM;
    }

    atomic_set(&mmap_buffer->write_idx, 0);
    atomic_set(&mmap_buffer->read_idx, 0);
    atomic_set(&mmap_buffer->num_messages, 0);

    ant_print_message(ANT_KERNEL_MODULE_NAME, "Module loaded.");
    ant_cpp_greet("Kernel");

    send_mmap_message("Hello from kernel module init!");
    msleep(100);
    send_mmap_message("This is a second message from kernel.");
    msleep(100);
    send_mmap_message("Kernel is sending messages via mmap.");

    return 0;
}

static void __exit kernel_module_exit(void) {
    dev_t dev_num = MKDEV(major_number, 0);

    if (mmap_buffer) {
        kfree(mmap_buffer);
        mmap_buffer = NULL;
    }

    cdev_del(&mmap_cdev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "%s: Module unloaded.\n", ANT_KERNEL_MODULE_NAME);
}

module_init(kernel_module_init);
module_exit(kernel_module_exit);
