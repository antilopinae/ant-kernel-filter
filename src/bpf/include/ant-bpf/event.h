#ifndef EVENT_H
#define EVENT_H

#ifdef __BPF__
// part for bpf
#else
// part for userspace
#endif

struct event {
    __u64 timestamp_ms;     // time from boot
    __u32 pid;              // process id
    __u32 tgid;             // thread group id = process pid
    __u32 ppid;             // parent process id
    __u32 pgid;             // process group id = pid of leader group
    __u32 uid;              // user id
    __u32 gid;              // group id
    char comm[16];          // short name of process
    char cmdline[256];      // args or part of cmdline
} __attribute__((aligned(8)));

#endif // EVENT_H
