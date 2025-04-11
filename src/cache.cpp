#include "cache.hpp"
#include "intercon.hpp"
#include <iostream>

namespace csim
{

    void Caches::requestFromProcessor(MemReq memreq)
    {
        if (isReqAHit(memreq))
        {
            // std::cout << "processor " << memreq.proc_ << " " << memreq.inst_.command << " " << memreq.inst_.address << " cache hit" << std::endl;
            // request from processor is a hit, respond to processor and return.
            memreq.processor_->requestCompleted(memreq);
            return;
        }

        // request is a miss. Begin coherence stuff.
        switch (coherproto_)
        {
        case MI:
            break;
        case MSI:
            break;
        case MESI:
            break;
        default:
            break;
        }

        // make request on bus
        BusMsgType bus_msg_type = (memreq.inst_.command == OperationType::MEM_LOAD) ? BusMsgType::BUSREAD : BusMsgType::BUSWRITE;
        BusMsg bus_msg = {.type = bus_msg_type, .state = NONE, .memreq = memreq, .cache = this, .src_proc_ = memreq.proc_};
        intercon_->requestFromCache(bus_msg);

        // save request waiting for response.
        uint8_t proc = memreq.proc_;
        pending_requests_[proc] = memreq;
    }

    void Caches::requestFromBus(BusMsg bus_msg)
    {
        switch (coherproto_)
        {
        case MI:
            break;
        case MSI:
            break;
        case MESI:
            break;
        default:
            break;
        }
    }

    void Caches::replyFromBus(BusMsg bus_msg)
    {
        switch (coherproto_)
        {
        case MI:
            break;
        case MSI:
            break;
        case MESI:
            break;
        default:
            break;
        }
    }

    Caches::Caches(int num_procs, SnoopIntercon *intercon, CoherenceProtocol coherproto) : num_procs_(num_procs), intercon_(intercon), coherproto_(coherproto)
    {
        pending_requests_ = std::vector<std::optional<MemReq>>(num_procs_, std::nullopt);
        caches_ = std::vector(num_procs_, Cache{});
    }

    bool Caches::isReqAHit(MemReq &memreq)
    {
        uint64_t address = memreq.inst_.address;
        OperationType reqtype = memreq.inst_.command;

        uint8_t proc = memreq.proc_;

        Cache &cache = caches_.at(proc);
        if (cache.lines.find(address) == cache.lines.end())
        {
            // doesn't exist in cache.
            return false;
        }

        Line &line = cache.lines[address];

        if (reqtype == OperationType::MEM_LOAD)
        {
            return (line.coherstate == CoherenceStates::EXCLUSIVE || line.coherstate == CoherenceStates::MODIFIED || line.coherstate == CoherenceStates::SHARED);
        }
        else // A store
        {
            return (line.coherstate == CoherenceStates::MODIFIED);
        }

        return false;
    }

    void Caches::tick()
    {
        intercon_->tick();
    }
}