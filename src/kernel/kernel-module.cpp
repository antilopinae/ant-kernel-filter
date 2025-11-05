#include <iostream>
#include <string>
#include <vector>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/sysmacros.h>
#include <utility>
#include <memory>
#include <functional>

#include <ant-kernel/headers.h>
#include <log/logger.hpp>
#include <fmt/format.h>

#include <kernel-module.hpp>

namespace ant::kernel {
    KernelModule::KernelModule(std::string module_path, std::string module_name) noexcept : module_path_(
            std::move(module_path)),
        module_name_(std::move(module_name)), status_(KernelModuleStatus::kNotLoaded) {
    }

    auto KernelModule::Load() -> std::expected<KernelModuleStatus, core::Error> {
        std::string cmd = "insmod " + module_path_;

        log::Debug(fmt::format("Loading kernel module: {}", cmd));

        if (int ret = system(cmd.c_str()); ret != 0) {
            auto err = fmt::format("Failed to load kernel module: {}, error code: {}", module_path_, ret);
            log::Error(err);
            return std::unexpected{core::Error(err)};
        }

        status_ = KernelModuleStatus::kLoaded;
        return status_;
    }

    auto KernelModule::Unload() -> std::expected<KernelModuleStatus, core::Error> {
        std::string cmd = "rmmod " + module_name_;

        log::Debug(fmt::format("Unloading kernel module: {}", cmd));

        if (int ret = system(cmd.c_str()); ret != 0) {
            auto err = fmt::format("Failed to unload kernel module: {}, error code: {}", module_name_, ret);
            log::Error(err);
            return std::unexpected{core::Error(err)};
        }

        status_ = KernelModuleStatus::kNotLoaded;
        return status_;
    }

    int main(int argc, char *argv[]) {
        std::string module_path = argv[1];
        std::string device_node_path = argv[2];

        int major_num = 240;
        std::cout << "Creating device node (if needed): mknod " << device_node_path << " c " << major_num << " 0" <<
                std::endl;
        if (mknod(device_node_path.c_str(), S_IFCHR | 0666, makedev(major_num, 0)) == -1) {
            if (errno == EEXIST) {
                std::cout << "Device node " << device_node_path << " already exists." << std::endl;
            } else {
                std::perror("Failed to create device node");
                syslog(LOG_ERR, "Failed to create device node %s: %s", device_node_path.c_str(), strerror(errno));
                unload_kernel_module(ANT_KERNEL_MODULE_NAME);
                closelog();
                return 1;
            }
        } else {
            std::cout << "Device node " << device_node_path << " created successfully." << std::endl;
        }

        sleep(1);

        int fd = open(device_node_path.c_str(), O_RDWR);
        if (fd < 0) {
            std::perror("Failed to open device");
            syslog(LOG_ERR, "Failed to open device %s: %s", device_node_path.c_str(), strerror(errno));
            unload_kernel_module(ANT_KERNEL_MODULE_NAME);
            closelog();
            return 1;
        }

        mmap_buffer_t *mapped_buffer = (mmap_buffer_t *) mmap(NULL, ANT_MMAP_BUF_SIZE, PROT_READ | PROT_WRITE,
                                                              MAP_SHARED,
                                                              fd,
                                                              0);
        if (mapped_buffer == MAP_FAILED) {
            std::perror("Failed to mmap device");
            syslog(LOG_ERR, "Failed to mmap device %s: %s", device_node_path.c_str(), strerror(errno));
            close(fd);
            unload_kernel_module(ANT_KERNEL_MODULE_NAME);
            closelog();
            return 1;
        }

        std::cout << "Successfully mmap'd buffer at " << mapped_buffer << std::endl;
        syslog(LOG_INFO, "Successfully mmap'd kernel buffer.");

        int num_messages_in_buf = ANT_MMAP_BUF_SIZE / sizeof(mmap_message_t);
        std::cout << "Max messages in buffer: " << num_messages_in_buf << std::endl;

        std::cout << "Starting message polling. Press Ctrl+C to exit." << std::endl;
        while (true) {
            int current_read_idx = mapped_buffer->read_idx.load(std::memory_order_acquire);
            int current_write_idx = mapped_buffer->write_idx.load(std::memory_order_acquire);
            int num_available = mapped_buffer->num_messages.load(std::memory_order_acquire);

            // std::cout << "R: " << current_read_idx << ", W: " << current_write_idx << ", N: " << num_available << std::endl;

            if (num_available > 0) {
                mmap_message_t *msg_ptr = &mapped_buffer->messages[current_read_idx % num_messages_in_buf];

                std::atomic_thread_fence(std::memory_order_acquire);
                if (msg_ptr->ready.load(std::memory_order_relaxed)) {
                    syslog(LOG_INFO, "Kernel message: %s", msg_ptr->data);
                    std::cout << "Received from kernel: " << msg_ptr->data << std::endl;

                    msg_ptr->ready.store(0, std::memory_order_release);
                    mapped_buffer->num_messages.fetch_sub(1, std::memory_order_release);
                    mapped_buffer->read_idx.fetch_add(1, std::memory_order_release);
                }
            }
            sleep(1);
        }

        // munmap(mapped_buffer, ANT_MMAP_BUF_SIZE);
        // close(fd);
        // unload_kernel_module(ANT_KERNEL_MODULE_NAME);
        // closelog();
        return 0;
    }
}
