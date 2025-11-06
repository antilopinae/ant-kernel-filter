#ifndef ANT_MMAP_H
#define ANT_MMAP_H

#define ANT_DRIVER_NAME "ant_kernel_module"
#define ANT_MMAP_BUF_SIZE (4 * 4096)  // 16 kb
#define ANT_MAX_MSG_LEN 256

#ifdef __KERNEL__

#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/stddef.h>

typedef struct mmap_message {
    char data[ANT_MAX_MSG_LEN];
    u32 len;
    atomic_t ready; // 1 = ready, 0 = not
} mmap_message_t;

// Общий буфер
typedef struct mmap_buffer {
    atomic_t write_idx; // index for kernel write
    atomic_t read_idx; // index for user read
    atomic_t num_messages; // num messages
    mmap_message_t messages[0]; // array of messages
} mmap_buffer_t;

#else

#include <cstdint>
#include <atomic>

typedef struct {
    char data[ANT_MAX_MSG_LEN];
    uint32_t len;
    std::atomic<int32_t> ready;
} mmap_message_t;

typedef struct {
    std::atomic<int32_t> write_idx;
    std::atomic<int32_t> read_idx;
    std::atomic<int32_t> num_messages;
    mmap_message_t messages[0];
} mmap_buffer_t;

#endif  // __KERNEL__

#endif  // ANT_MMAP_H
