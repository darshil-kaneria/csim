#include "cpu.hpp"
#include "cache.hpp"
#include <iostream>

namespace csim
{
    
    CPU::CPU(TraceReader *trace_reader, uint8_t num_procs = 8, Cache *cache) : trace_reader_(trace_reader), num_procs_(num_procs), cache_(cache)
    {
        cycles_ = 0;
        proc_pending_mem_ = std::vector<bool>(num_procs_, false);
        proc_seq_ = std::vector<uint64_t>(num_procs_, 0);
        should_stop_ = false;
    }

    bool CPU::tick()
    {
        cache_->tick();

        should_stop_ = false;
        for (uint8_t proc = 0; proc < num_procs_; proc++)
        {
            if (proc_pending_mem_[proc])
            {
                // processor is waiting on memory, nothing to do in this tick.
                should_stop_ = true;
                continue;
            }

            // Processor is not waiting on memory

            // get next instruction to process
            Instruction inst = trace_reader_->readNextLine(proc);

            // TODO: check for eof

            // get the processor specific sequence no
            uint64_t seq = proc_seq_[proc]++;

            // pass instruction to cache
            MemReq mem_request(inst, proc, seq, this);
            proc_pending_reqs_[proc] = mem_request;
            proc_pending_mem_[proc] = true;
            cache_->requestFromProcessor(mem_request);
        }
    }

   
    void CPU::requestCompleted(MemReq memreq)
    {
        if (proc_pending_reqs_[memreq.proc_] == memreq)
        {
            proc_pending_mem_[memreq.proc_] = false;
        }
        else
        {
            std::cerr << "Trying to complete an invalid request" << std::endl;
        }
    }

} // namespace csim
