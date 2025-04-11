#include "cpu.hpp"
#include "cache.hpp"
#include <iostream>
#include <cassert>

namespace csim
{

    CPU::CPU(TraceReader *trace_reader, uint8_t num_procs, Caches *cache) : trace_reader_(trace_reader), num_procs_(num_procs), cache_(cache)
    {
        cycles_ = 0;
        proc_seq_ = std::vector<uint64_t>(num_procs_, 0);
        proc_pending_reqs_ = std::vector<std::optional<MemReq>>(num_procs_, std::nullopt);
    }

    bool CPU::tick()
    {
        cache_->tick();

        bool still_running = false;
        for (uint8_t proc = 0; proc < num_procs_; proc++)
        {
            if (proc_pending_reqs_[proc])
            {
                // processor is waiting on memory, nothing to do in this tick.
                still_running = true;
                continue;
            }

            // Processor is not waiting on memory

            // get next instruction to process
            std::optional<Instruction> inst = trace_reader_->readNextLine(proc);

            // check for eof
            if (!inst)
            {
                continue;
            }

            // get the processor specific sequence no
            uint64_t seq = proc_seq_[proc]++;

            // pass instruction to cache
            MemReq mem_request(*inst, proc, seq, this);
            proc_pending_reqs_[proc] = mem_request;
            cache_->requestFromProcessor(mem_request);
            still_running = true;
        }
        return still_running;
    }

    void CPU::requestCompleted(MemReq memreq)
    {
        assert(proc_pending_reqs_[memreq.proc_].has_value());

        if (proc_pending_reqs_[memreq.proc_].value() == memreq)
        {
            proc_pending_reqs_[memreq.proc_].reset();
        }
        else
        {
            std::cerr << "Trying to complete an invalid request" << std::endl;
        }
    }

} // namespace csim
