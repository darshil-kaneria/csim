#include "bus.hpp"
#include "globals.hpp"
#include "cache.hpp"
#include <cassert>

namespace csim
{
    void SnoopBus::tick()
    {
        // process a request from bus.
        assert(!curr_msg_);

        if (inputq_.empty()) {
            return;
        }

        BusMsg busreq = inputq_.front();
        inputq_.pop();

        assert(busreq.type_ == BUSREAD || busreq.type_ == BUSWRITE || busreq.type_ == BUSUPGRADE);

        // send to all processors except source
        size_t src = busreq.src_proc_;

        curr_msg_->busmsg = busreq;
        curr_msg_->state = PROCESSING;

        for (size_t proc = 0; proc < num_proc_; proc++)
        {
            if (src != proc)
            {
                BusMsg msg = curr_msg_->busmsg;
                msg.dst_proc_ = proc;
                caches_->requestFromBus(msg);
            }
        }

        BusMsg busresp;
        busresp = curr_msg_->busmsg;
        busresp.dst_proc_ = src;

        if (curr_msg_->state == CACHEDATA)
        {
            // cache flushed data
            busresp.type_ = BUSDATA;
        }
        else if (curr_msg_->state == CACHESHARED)
        {
            // cache provided data in shared mode
            busresp.type_ = BUSSHARED;
        }
        else
        {
            assert(curr_msg_->state == PROCESSING);
            // cache didn't provide data, memory will
            busresp.type_ = MEMDATA;
        }
        caches_->replyFromBus(busresp);
        curr_msg_.reset();
    }

    SnoopBus::SnoopBus(size_t num_proc, Caches *caches) : num_proc_(num_proc), caches_(caches)
    {
        inputq_ = std::queue<BusMsg>();
    }

    void SnoopBus::requestFromCache(BusMsg busmsg)
    {
        // received request from cache.
        // can be rd/wr/upg
        assert(busmsg.type_ == BUSREAD || busmsg.type_ == BUSWRITE || busmsg.type_ == BUSUPGRADE);
        assert(inputq_.size() <= num_proc_);
        inputq_.push(busmsg);
    }

    void SnoopBus::replyFromCache(BusMsg busmsg)
    {
        // received reply from cache.
        // can be data/shared
        assert(curr_msg_);
        assert(busmsg.type_ == BUSDATA || busmsg.type_ == BUSSHARED);
        if (busmsg.type_ == BUSDATA)
        {
            curr_msg_->state = CACHEDATA;
        }
        else
        {
            curr_msg_->state = CACHESHARED;
        }
    }

    void SnoopBus::setCaches(Caches *caches)
    {
        caches_ = caches;
    }
}