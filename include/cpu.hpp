#pragma once

#include "trace.hpp"
#include <vector>
#include "busmsg.hpp"

namespace csim
{
    class Caches;
    class TraceReader;

    struct CPU
    {
        size_t seq_;                        // Strictly Increasing sequence number per processor.
        std::optional<CPUMsg> pending_req_; // current request being processed.
    };

    class CPUS
    {
    public:
        CPUS(TraceReader *trace_reader, size_t num_procs, Caches *cache);
        bool tick();

        void replyFromCache(CPUMsg cpumsg);
        void setCaches(Caches *caches);

    private:
        TraceReader *trace_reader_; // Trace reader to read traces
        size_t num_procs_;          // Number of processes.
        Caches *caches_;            // The caches in the system.
        std::vector<CPU> cpus;
    };

}