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
        std::optional<BusMsg> curr_req_;    // current request being processed.
        std::priority_queue<BusMsg> inputq; // input queue to receive messages
    };

    class CPUS
    {
    public:
        /**
         * @brief Advances CPU by 1 tick/cycle.
         *
         * CPU calls cache tick first.
         * Afterwards, it checks if it can process for any processor, reads tracefile and sends request to cache component.
         */
        bool tick();
        CPUS(TraceReader *trace_reader, size_t num_procs, Caches *cache);

    private:
        TraceReader *trace_reader_; // Trace reader to read traces
        size_t num_procs_;          // Number of processes.
        Caches *cache_;             // The caches in the system.
        std::vector<CPU> cpus;
        size_t cycles_;                     // The cycle count.

    };

}