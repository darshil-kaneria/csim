#include "cache.hpp"
#include "intercon.hpp"

namespace csim
{
    // Cache::Cache(uint32_t set_assoc, uint32_t block_size, uint32_t tag_size, uint32_t addr_len)
    // : set_assoc_(set_assoc),
    // block_size_(block_size),
    // tag_size_(tag_size),
    // addr_len_(addr_len) {
    //     // Todo...
    // }
    void Caches::requestFromProcessor(MemReq memreq)
    {
        if (isReqAHit(memreq))
        {
            // request from processor is a hit, enque request in done queue to notify processor in next tick.
            uint8_t proc = memreq.proc_;
            done_requests_[proc] = memreq;
            return;
        }

        // request is a miss. Begin coherence stuff.

        // make request on bus
        BusMsgType bus_msg_type = (memreq.inst_.command == OperationType::MEM_LOAD) ? BusMsgType::BUSREAD : BusMsgType::BUSWRITE;
        BusMsg bus_msg = {.type = bus_msg_type, .state = NONE, .memreq = memreq, .cache = this};
        intercon->SendBusMessage(bus_msg);

        // save request waiting for response.
        uint8_t proc = memreq.proc_;
        pending_requests_[proc] = memreq;
    }

    void Caches::trafficFromBus(BusMsg bus_msg)
    {
    }

    void Caches::dataProvidedFromBus(BusMsg bus_msg)
    {
    }

    bool Caches::isReqAHit(MemReq &memreq)
    {
        // TODO check cache if it is a hit
        return false;
    }

    void Caches::tick()
    {
        intercon->tick();

        // process all done requests
        for (int proc = 0; proc < num_procs_; proc++) {
            if (done_requests_[proc]) {
                MemReq &done_request = done_requests_[proc].value();
                done_request.processor_->requestCompleted(done_request);
                done_requests_[proc] = std::nullopt;
            }
        }
      
    }
}