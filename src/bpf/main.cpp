#include <algorithm>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <memory>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>

#include <bpf/bpf.h>
#include <bpf/libbpf.h>

#include <nlohmann/json.hpp>

#include <linux/types.h>

#include <bpf/event.h>

#include BPF_SKEL_HEADER

#define ANT_CONCAT3(a, b, c) a##b##c
#define ANT_CONCAT(a, b, c) ANT_CONCAT3(a, b, c)

#define ANT_BPF_PROG_NAME ant_bpf_bpf
#define ANT_BPF_FUNC_CONCAT(name) ANT_CONCAT(ANT_BPF_PROG_NAME, __, name)

namespace ProcMon {
    class SignalGuard {
    private:
        static inline std::atomic<bool> running{false};

    public:
        SignalGuard() {
            running.store(true);
            std::signal(SIGINT, &SignalGuard::handle);
            std::signal(SIGTERM, &SignalGuard::handle);
        }

        bool getRunning() const { return running.load(); }

        ~SignalGuard() {
            std::signal(SIGINT, SIG_DFL);
            std::signal(SIGTERM, SIG_DFL);
        }

    private:
        static void handle(int) { running.store(false); }
    };

    class EventBuffer {
    private:
        std::vector<event> eventBuffer;
        std::mutex eventMut;

    public:
        void push(const event &ev) {
            std::lock_guard lock(eventMut);
            eventBuffer.push_back(ev);
        }

        std::vector<event> takeAll() {
            std::lock_guard lock(eventMut);
            std::vector<event> tmp;

            // there may be an unpleasant moment when we copy the vector every 100 ms,
            // sort it and dump it into a file;
            // data from the processes may not arrive in the package sooner
            // and will arrive in the package later, which leads to incorrect sorting
            // and incorrect writing to the file
            // but let's skip this moment for the sake of brevity and efficiency of the code
            tmp.swap(eventBuffer);
            return tmp;
        }
    };

    class JsonDumper {
    private:
        std::ofstream outStream;

    public:
        explicit JsonDumper(const std::string &filename) : outStream(filename, std::ios::out) {
            if (!outStream)
                throw std::runtime_error("Cannot open " + filename);
        }

        void dump(std::vector<event> &&evs) {
            std::sort(evs.begin(), evs.end(),
                      [](auto &a, auto &b) {
                          return a.timestamp_ms < b.timestamp_ms;
                      }
            );

            for (const auto &ev: evs) {
                nlohmann::json j = {
                    {"pid", ev.pid},
                    {"tgid", ev.tgid},
                    {"ppid", ev.ppid},
                    {"pgid", ev.pgid},
                    {"uid", ev.uid},
                    {"gid", ev.gid},
                    {"comm", std::string(ev.comm)},
                    {"cmdline", std::string(ev.cmdline)},
                    {"timestamp", ev.timestamp_ms}
                };

                outStream << j.dump() << "\n";
            }

            outStream.flush();
        }
    };

    class BPFProgram {
    private:
        struct ANT_BPF_PROG_NAME *pmSkel{nullptr};
        struct perf_buffer *pmBuffer{nullptr};
        EventBuffer &eventBuffer;

    public:
        explicit BPFProgram(EventBuffer &buf) : eventBuffer(buf) {
            if (pmSkel = ANT_BPF_FUNC_CONCAT(open_and_load)(); !pmSkel)
                throw std::runtime_error("Failed to open BPF skeleton");

            // tracepoint/sched/sched_process_exec
            if (int err = ANT_BPF_FUNC_CONCAT(attach)(pmSkel); err) {
                ANT_BPF_FUNC_CONCAT(destroy)(pmSkel);
                throw std::runtime_error("Failed to attach BPF: " + std::to_string(err));
            }

            int map_fd = bpf_map__fd(pmSkel->maps.events);

            //     for each CPU_ID from the range 0 .. max_entries-1
            //     do perf_event_open() on this CPU,
            //     subscribes to read events
            //     and creates epoll or poll processing
            if (pmBuffer = perf_buffer__new(
                    map_fd, // fd map
                    8, // buffer with 8 pages on CPU
                    &BPFProgram::receiveEvent, // callback for event
                    &BPFProgram::receiveLost, // callback for lost
                    this, // user context (this)
                    nullptr); // options
                !pmBuffer) {
                ANT_BPF_FUNC_CONCAT(destroy)(pmSkel);
                throw std::runtime_error("Failed to create perf buffer: " + std::string(strerror(errno)));
            }
        }

        void poll(int interval_ms) {
            if (int err = perf_buffer__poll(pmBuffer, interval_ms); err < 0 && errno != EINTR)
                //  if not interrupted system call
                throw std::runtime_error("perf_buffer__poll error: " + std::string(strerror(errno)));
        }

        ~BPFProgram() {
            if (pmBuffer)
                perf_buffer__free(pmBuffer);

            if (pmSkel)
                ANT_BPF_FUNC_CONCAT(destroy)(pmSkel);
        }

    private:
        static void receiveEvent(void *ctx, int cpu, void *data, __u32 sz) {
            auto self = static_cast<BPFProgram *>(ctx);
            auto ev = *static_cast<event *>(data);
            self->eventBuffer.push(ev);
        }

        static void receiveLost(void *ctx, int cpu, __u64 cnt) {
            std::cerr << "Lost " << cnt << " events on CPU " << cpu << "\n";
        }
    };

    class App {
    public:
        explicit App(std::string &&dumpFilename_)
            : dumpFilename(std::forward<std::string>(dumpFilename_)) {
        }

        void run() {
            SignalGuard signals;
            BPFProgram bpf(eventBuffer);
            JsonDumper dumper(dumpFilename);

            std::cout << "Running Process Monitor... Press Ctrl+C to stop.\n";

            while (signals.getRunning()) {
                bpf.poll(100); // poll events
                auto evs = eventBuffer.takeAll(); // take received events

                if (!evs.empty())
                    dumper.dump(std::move(evs)); // dump to file
            }

            std::cout << "Exiting, output written to: " << dumpFilename << "\n";
        }

    private:
        EventBuffer eventBuffer;
        std::string dumpFilename;
    };
} // namespace ProcMon

int main(int argc, char *argv[]) {
    std::string outputFile = (argc > 1) ? argv[1] : "processes.json";
    try {
        ProcMon::App app{std::move(outputFile)};
        app.run();
        return 0;
    } catch (const std::exception &ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
}
