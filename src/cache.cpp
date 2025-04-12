#include "cache.hpp"
#include "globals.hpp"
#include "bus.hpp"
#include <cassert>

namespace csim
{
    void Caches::tick()
    {
        // validate consistency in our cache.

        snoopbus_->tick();
    }

    void Caches::requestFromProcessor(CPUMsg cpureq)
    {
        size_t proc = cpureq.proc_;
        Cache &cache = caches_[proc];
        if (cache.isAHit(cpureq))
        {
            CPUMsg cpuresp = cpureq;
            cpuresp.msgtype = CPUMsgType::RESPONSE;
            cpus_->replyFromCache(cpuresp);
        }

        assert(!cache.pending_bus_msg);

        BusMsg busreq = BusMsg{
            .proc_cycle_ = cycle,
            .type_ = (cpureq.inst_.command == OperationType::MEM_LOAD) ? BusMsgType::BUSREAD : BusMsgType::BUSWRITE,
            .cpureq_ = cpureq,
            .src_proc_ = proc,
            .dst_proc_ = BROADCAST,
        };
        snoopbus_->requestFromCache(busreq);
        cache.pending_bus_msg = busreq;
    }

    void Caches::requestFromBus(BusMsg busmsg)
    {
        // another processor sent a request.
        // can be rd/wr/upgr
        // respond if you have it, depends on snooping protocol.

        switch (coherproto_)
        {
        case MI:
        case MSI:
        case MESI:
            break;
        }
    }

    void Caches::replyFromBus(BusMsg busmsg)
    {
        // another processor/Memory sent a reply.
        // can be data/shared/memory.
        // depends on coherence protocol.

        switch (coherproto_)
        {
        case MI:
        case MSI:
        case MESI:
            break;
        }
    }

    Caches::Caches(size_t num_procs, SnoopBus *snoopbus, CPUS *cpus, CoherenceProtocol coherproto) : num_procs_(num_procs), snoopbus_(snoopbus), cpus_(cpus), coherproto_(coherproto)
    {
        caches_ = std::vector(num_procs_, Cache{.lines = std::unordered_map<size_t, Line>(), .pending_bus_msg = std::nullopt, .coherproto_ = coherproto_});
    }

    void Caches::setBus(SnoopBus *snoopbus)
    {
        snoopbus_ = snoopbus;
    }

    void Caches::setCPUs(CPUS *cpus)
    {
        cpus_ = cpus;
    }

    bool Cache::isAHit(CPUMsg &cpureq)
    {
        // TODO: might depend on coherence protocol
        switch (coherproto_)
        {
        case MI:
        case MSI:
        case MESI:
            break;
        }
        return true;
    }
}
