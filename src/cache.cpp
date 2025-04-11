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
            // request from processor is a hit, respond to processor and return.
            memreq.processor_->requestCompleted(memreq);
            return;
        }

        // request is a miss. Begin coherence stuff.

        // make request on bus
        BusMsgType bus_msg_type = (memreq.inst_.command == OperationType::MEM_LOAD) ? BusMsgType::BUSREAD : BusMsgType::BUSWRITE;
        BusMsg bus_msg = {.type = bus_msg_type, .state = NONE, .memreq = memreq, .cache = this, .src_proc_=memreq.proc_};
        intercon_->requestFromCache(bus_msg);

        // save request waiting for response.
        uint8_t proc = memreq.proc_;
        pending_requests_[proc] = memreq;
    }

    void Caches::requestFromBus(BusMsg bus_msg)
    {
    }

    void Caches::replyFromBus(BusMsg bus_msg)
    {
    }

    Caches::Caches(int num_procs, SnoopIntercon *intercon) : num_procs_(num_procs), intercon_(intercon)
    {
        pending_requests_ = std::vector<std::optional<MemReq>>(num_procs_, std::nullopt);
        caches_ = std::vector(num_procs_, Cache{});
    }

    bool Caches::isReqAHit(MemReq &memreq)
    {
        // TODO check cache if it is a hit
        return false;
    }

    void Caches::tick()
    {
        intercon_->tick();
    }
    Cache::Cache(uint32_t set_assoc, uint32_t block_size, uint32_t tag_size, uint32_t addr_len)
    {
    }
    Cache::~Cache()
    {
    }
}