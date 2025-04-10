#pragma once

#include "trace.hpp"
#include <vector>
#include "memreq.hpp"

namespace csim
{
    class Caches;
    class TraceReader;

    class CPU
    {
    public:
        /**
         * @brief CPU constructor
         * @param trace_reader The trace reader that supplies instructions to the CPU to process.
         * @param num_procs The number of processes. Default is set to 8.
         * @param cache A pointer to the cache component.
         */
        CPU(TraceReader *trace_reader, uint8_t num_procs, Caches *cache);
        /**
         * @brief Advances CPU by 1 tick/cycle.
         *
         * CPU calls cache tick  first.
         * Afterwards, it checks if it can process for any processor, reads tracefile and sends request to cache component.
         */
        bool tick();

        /**
         * @brief Called by cache when the request is completed.
         *
         * @param memreq The pending memory request the cache has processed.
         * @return
         */
        void requestCompleted(MemReq memreq); // Function called when a pending memory request is completed

    private:
        TraceReader *trace_reader_;             // Trace reader to read traces
        uint8_t num_procs_ = 8;                 // Number of processes.
        uint64_t cycles_;                       // The cycle count.
        Caches *cache_;                         // The caches in the system.
        std::vector<bool> proc_pending_mem_;    // True if the processor is waiting on a memory request.
        std::vector<uint64_t> proc_seq_;        // Strictly Increasing sequence number per processor
        std::vector<MemReq> proc_pending_reqs_; // Pending memory request for each processor.
        bool should_stop_ = false;              // Whether processor should stop executing
    };
}