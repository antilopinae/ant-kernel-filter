#include "vmlinux.h"

#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_tracing.h>

#include "include/bpf/event.h"

struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);    //  type
    __uint(key_size, sizeof(u32));                  //  cpu id
    __uint(value_size, sizeof(u32));                //  fd for the core to send events to the cpu-N
    __uint(max_entries, 128);
} events SEC(".maps");

static __always_inline bool is_kernel_thread(struct task_struct *task) {
    // for kernel threads, the mm field is always NULL
    // because they do not operate in the context of user space
    // and do not have their own address space
    return BPF_CORE_READ(task, mm) == NULL; // *mm_struct = mm indicates using user memory
}

// triggered when a process successfully execs a new program
SEC("tracepoint/sched/sched_process_exec")
int handle_exec(struct trace_event_raw_sched_process_exec *ctx) {
    struct task_struct *task = (struct task_struct *)bpf_get_current_task();

    if (is_kernel_thread(task))
        return 0;

    struct event data = {};

    // bpf_get_current_pid_tgid returns a 64-bit number containing:
    u64 pid_tgid = bpf_get_current_pid_tgid();

    // in the lower 32 bits, the pid of the thread
    // in the upper 32 bits, the tgid (thread group id, process pid)
    data.pid = pid_tgid & 0xFFFFFFFF;
    data.tgid = pid_tgid >> 32;

    // parent process id
    data.ppid = BPF_CORE_READ(task, real_parent, tgid);
    // pgid = process group ID = pid of the process that became the leader of this group
    data.pgid = BPF_CORE_READ(task, group_leader, pid);

    // bpf_get_current_uid_gid returns a 64-bit number containing:
    u64 uid_gid = bpf_get_current_uid_gid();

    // low 32 bits = UID (user ID) of the current process
    // high 32 bits = GID (group ID) of the current process
    data.uid = uid_gid & 0xFFFFFFFF;
    data.gid = uid_gid >> 32;

    // monotonic timestamp in milliseconds (since boot)
    data.timestamp_ms = bpf_ktime_get_ns() / 1000000;

    // copy short name of the executable into data.comm
    bpf_get_current_comm(&data.comm, sizeof(data.comm));

    struct mm_struct *mm = BPF_CORE_READ(task, mm);

    // read command-line arguments from user space (usually argv[0])
    unsigned long arg_start = BPF_CORE_READ(mm, arg_start);

    // safe-read str from userspace
    if (bpf_probe_read_user_str(&data.cmdline, sizeof(data.cmdline), (const void *)arg_start) < 0) {
        // error while reading str
        data.cmdline[0] = '\0';
    }

    // send data to user space via perf buffer
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &data, sizeof(data));

    return 0;
}

char LICENSE[] SEC("license") = "GPL"; // .license = GNU General Public License (GPL-only BPF helpers)
