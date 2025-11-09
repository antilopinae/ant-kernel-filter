/*
 * example_mutex.c
 */
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/printk.h>

static DEFINE_MUTEX(mymutex);

static int __init example_mutex_init(void)
{
    int ret;

    pr_info("example_mutex init\n");

    ret = mutex_trylock(&mymutex);
    /*
        mutex_lock_interruptible()
        behave similarly to mutex_lock() but allow the waiting process to be interrupted by signals.
        If a task receives a signal (like a termination signal) while waiting for the lock,
        it will exit the waiting state and return an error code ( -EINTR ).
        This is useful for applications that need to handle external events even while waiting for a lock.

        Functions like mutex_lock_nested and mutex_lock_interruptible_nested() incorporate the __nested() functionality,
        providing support for nested locking.
        This prior locking mechanism aids in managing lock acquisition and preventing deadlocks,
        often employing a subclass parameter for more precise deadlock detection.
    */
    if (ret != 0) {
        pr_info("mutex is locked\n");

        if (mutex_is_locked(&mymutex) == 0)
            pr_info("The mutex failed to lock!\n");

        mutex_unlock(&mymutex);
        pr_info("mutex is unlocked\n");
    } else
        pr_info("Failed to lock\n");

    return 0;
}

static void __exit example_mutex_exit(void)
{
    pr_info("example_mutex exit\n");
}

module_init(example_mutex_init);
module_exit(example_mutex_exit);

MODULE_DESCRIPTION("Mutex example");
MODULE_LICENSE("GPL");
