#pragma once

#include "trace.hpp"
#include <vector>
#include "busmsg.hpp"

namespace csim
{
    class SnoopCaches;
    struct TraceReader;
    class DirectoryCaches;

    struct CPU
    {
        size_t seq_;                        // Strictly Increasing sequence number per processor.
        std::optional<CPUMsg> pending_req_; // current request being processed.
    };

    class CPUS
    {
    public:
        CPUS(TraceReader *trace_reader, size_t num_procs, DirectoryCaches* cache);
        bool cycle();

        void replyFromCache(CPUMsg cpumsg);
        void setCaches(DirectoryCaches*caches);

    private:
        TraceReader *trace_reader_; // Trace reader to read traces
        size_t num_procs_;          // Number of processes.
        DirectoryCaches*caches_;       // The snoop caches in the system.
        std::vector<CPU> cpus;
    };

}