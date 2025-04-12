#include "cache.hpp"
#include "globals.hpp"
#include "bus.hpp"
#include <cassert>

namespace csim
{
    void Caches::tick()
    {
        // process any messages received from cpu
        for (size_t proc = 0; proc < num_procs_; proc++)
        {
            Cache &cache = caches_[proc];
            while (!cache.inputqcpu.empty() && cache.inputqcpu.top().proc_cycle_ <= cycle)
            {
                // within cycle, process request from CPU.
                const CPUMsg &cpureq = cache.inputqcpu.top();

                if (cache.isAHit(cpureq))
                {
                    // if a hit respond immediately
                    CPUMsg cpuresp = cpureq;
                    cpuresp.msgtype = CPUMsgType::RESPONSE;
                    cpus_->enqueueMsgFromCache(cpuresp, proc);
                }
                else
                {
                    // send a bus request
                    BusMsg busreq = BusMsg{
                        .proc_cycle_ = cycle + BUSDELAY,
                        .type_ = (cpureq.inst_.command == OperationType::MEM_LOAD) ? BusMsgType::BUSREAD : BusMsgType::BUSWRITE,
                        .cpureq_ = cpureq,
                        .src_proc_ = proc,
                        .dst_proc_ = BROADCAST,
                    };
                    snoopbus_->enqueMsg(busreq);

                    // save message as pending
                    assert(!cache.pendingmsg);
                    cache.pendingmsg = busreq;
                }
                cache.inputqcpu.pop();
            }
        }

        // process any messages received from bus
        for (size_t proc = 0; proc < num_procs_; proc++)
        {
            Cache &cache = caches_[proc];

            while (!cache.inputqbus.empty() && cache.inputqbus.top().proc_cycle_ <= cycle)
            {
                // within cycle, processing request from bus.

                // if its a response to our pending request send response to cpu,
                switch (coherproto_)
                {
                case CoherenceProtocol::MI:
                case CoherenceProtocol::MSI:
                case CoherenceProtocol::MESI:
                    break;
                }

                // will depend on the snoop protocol used.

                cache.inputqbus.pop();
            }
        }

        // validate consistency in our cache.

        snoopbus_->tick();
    }

    void Caches::enqueueMsgFromProc(CPUMsg &cpumsg, size_t proc)
    {
        caches_[proc].inputqcpu.push(cpumsg);
    }

    void Caches::enqueueMsgFromBus(BusMsg &busmsg, size_t proc)
    {
        caches_[proc].inputqbus.push(busmsg);
    }

    Caches::Caches(size_t num_procs, SnoopBus *snoopbus, CPUS *cpus, CoherenceProtocol coherproto) : num_procs_(num_procs), snoopbus_(snoopbus), cpus_(cpus), coherproto_(coherproto)
    {
        caches_ = std::vector(num_procs_, Cache{
                                              .inputqbus = std::priority_queue<BusMsg, std::vector<BusMsg>, BusMsgComparator>(),
                                              .inputqcpu = std::priority_queue<CPUMsg, std::vector<CPUMsg>, CPUMsgComparator>(),
                                              .lines = std::unordered_map<size_t, Line>()});
    }

    void Caches::setBus(SnoopBus *snoopbus)
    {
        snoopbus_ = snoopbus;
    }

    void Caches::setCPUs(CPUS *cpus)
    {
        cpus_ = cpus;
    }

    bool Cache::isAHit(CPUMsg cpureq)
    {
        // TODO: might depend on coherence protocol
        return true;
    }

}
