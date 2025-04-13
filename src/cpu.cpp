#include "cpu.hpp"
#include "globals.hpp"
#include "cache.hpp"
#include <cassert>

namespace csim
{
    bool CPUS::cycle()
    {

        bool progress = false;

        for (size_t proc = 0; proc < num_procs_; proc++)
        {
            CPU &cpu = cpus[proc];

            if (cpu.pending_req_)
            {
                // stalling due to ongoing operation.
                progress = true;
                continue;
            }

            // processor not waiting on memory

            // ensure no pending requests
            assert(!cpu.pending_req_);

            // get next instruction
            std::optional<Instruction> inst = trace_reader_->readNextLine(proc);

            // check for eof
            if (!inst)
            {
                continue;
            }

            // send request to cache
            size_t seq = cpu.seq_++;

            CPUMsg cpumsg = CPUMsg{.proc_cycle_ = cycles,
                                   .msgtype = CPUMsgType::REQUEST,
                                   .inst_ = *inst,
                                   .proc_ = proc,
                                   .proc_seq_ = seq};
            cpu.pending_req_ = cpumsg;

            caches_->requestFromProcessor(cpumsg);

            progress = true;
        }

        caches_->cycle();

        cycles++;

        return progress;
    }

    void CPUS::replyFromCache(CPUMsg cpumsg)
    {
        size_t proc = cpumsg.proc_;
        CPU &cpu = cpus[proc];
        assert(cpu.pending_req_);
        assert(cpu.pending_req_.value() == cpumsg);
        cpu.pending_req_.reset();
    }

    void CPUS::setCaches(Caches *caches)
    {
        caches_ = caches;
    }

    CPUS::CPUS(TraceReader *trace_reader, size_t num_procs, Caches *caches) : trace_reader_(trace_reader), num_procs_(num_procs), caches_(caches)
    {
        cpus = std::vector(num_procs_, CPU{.seq_ = 0, .pending_req_ = std::nullopt});
    }
}