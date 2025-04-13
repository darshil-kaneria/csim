#include "cache.hpp"
#include "globals.hpp"
#include "bus.hpp"
#include <cassert>

namespace csim
{
    void Caches::cycle()
    {
        // validate consistency in our cache.

        // try to process request from processor.
        for (size_t proc = 0; proc < num_procs_; proc++)
        {
            Cache &cache = caches_[proc];

            if (!cache.pending_cpu_req)
                continue;

            // there is a pending request from processor
            if (cache.isAHit(cache.pending_cpu_req.value()))
            {
                CPUMsg cpuresp = cache.pending_cpu_req.value();
                cpuresp.msgtype = CPUMsgType::RESPONSE;
                cpus_->replyFromCache(cpuresp);
                cache.pending_cpu_req.reset();
            }
            else
            {
                // check if our turn to send on bus and send bus request
                if (snoopbus_->isCacheTurn(proc))
                {

                    // TODO: handle upgrades
                    BusMsg busreq = BusMsg{
                        .proc_cycle_ = cycles,
                        .type_ = (cache.pending_cpu_req->inst_.command == OperationType::MEM_LOAD) ? BusMsgType::BUSREAD : BusMsgType::BUSWRITE,
                        .cpureq_ = cache.pending_cpu_req.value(),
                        .src_proc_ = proc,
                        .dst_proc_ = BROADCAST,
                    };
                    snoopbus_->requestFromCache(busreq);
                }
            }
        }

        snoopbus_->cycle();
    }

    void Caches::requestFromProcessor(CPUMsg cpureq)
    {
        size_t proc = cpureq.proc_;
        Cache &cache = caches_[proc];
        assert(!cache.pending_cpu_req);
        cache.pending_cpu_req = cpureq;
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
        caches_ = std::vector(num_procs_, Cache{.lines = std::unordered_map<size_t, Line>(), .pending_cpu_req = std::nullopt, .coherproto_ = coherproto_});
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
