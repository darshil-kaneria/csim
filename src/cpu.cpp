#include "cpu.hpp"
#include "globals.hpp"
#include "cache.hpp"
#include <cassert>

namespace csim
{
    bool CPUS::tick()
    {
        // Process any done requests
        for (size_t proc = 0; proc < num_procs_; proc++)
        {
            CPU &cpu = cpus[proc];
            std::priority_queue<CPUMsg, std::vector<CPUMsg>, CPUMsgComparator> &inputq = cpu.inputq;

            while (!inputq.empty() && inputq.top().proc_cycle_ <= cycle)
            {
                CPUMsg cpumsg = inputq.top();

                assert(cpu.curr_req_);
                assert(cpumsg == cpu.curr_req_.value());

                inputq.pop();
                cpu.curr_req_.reset();
            }
        }

        bool progress = false;

        for (size_t proc = 0; proc < num_procs_; proc++)
        {
            CPU &cpu = cpus[proc];

            if (cpu.curr_req_)
            {
                // stalling due to ongoing operation.
                progress = true;
                continue;
            }

            // processor not waiting on memory
            // get next instruction
            std::optional<Instruction> inst = trace_reader_->readNextLine(proc);

            // check for eof
            if (!inst)
            {
                continue;
            }

            // enqueue request to cache
            size_t seq = cpu.seq_++;

            CPUMsg cpumsg = CPUMsg{.proc_cycle_ = cycle,
                                   .msgtype = CPUMsgType::REQUEST,
                                   .inst_ = *inst,
                                   .proc_ = proc,
                                   .proc_seq_ = seq};
            cpu.curr_req_ = cpumsg;

            caches_->enqueueMsgFromProc(cpumsg, proc);

            progress = true;
        }

        caches_->tick();

        cycle++;

        return progress;
    }

    void CPUS::enqueueMsgFromCache(CPUMsg cpumsg, size_t proc)
    {
        cpus[proc].inputq.push(cpumsg);
    }

    void CPUS::setCaches(Caches *caches)
    {
        caches_ = caches;
    }

    CPUS::CPUS(TraceReader *trace_reader, size_t num_procs, Caches *caches) : trace_reader_(trace_reader), num_procs_(num_procs), caches_(caches)
    {
        cpus = std::vector(num_procs_, CPU{
                                           .seq_ = 0,
                                           .curr_req_ = std::nullopt,
                                           .inputq = std::priority_queue<CPUMsg, std::vector<CPUMsg>, CPUMsgComparator>()});
    }
}