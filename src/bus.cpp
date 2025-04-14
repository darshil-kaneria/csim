#include "bus.hpp"
#include "globals.hpp"
#include "cache.hpp"
#include <cassert>

namespace csim
{
    void SnoopBus::cycle()
    {
        // process a request from bus.
        if (curr_msg_)
        {
            assert(curr_msg_->state == PROCESSING);

            std::cout << "Processing " << curr_msg_->busmsg << std::endl;

            BusMsg &busreq = curr_msg_->busmsg;
            assert(busreq.type_ == BUSREAD || busreq.type_ == BUSWRITE || busreq.type_ == BUSUPGRADE);

            // send to all processors except source
            size_t src = busreq.src_proc_;

            for (size_t proc = 0; proc < num_proc_; proc++)
            {
                if (src != proc)
                {
                    // record interconnect traffic
                    stats_->interconstats.traffic++;

                    BusMsg msg = busreq;
                    msg.dst_proc_ = proc;
                    caches_->requestFromBus(msg);
                }
            }

            // caches will change state if necessary. Now check state

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

                // simulate traffic for memory providing data.
                stats_->interconstats.traffic++;
            }
            // record interconnect traffic
            stats_->interconstats.traffic++;

            caches_->replyFromBus(busresp);

            std::cout << "Processed " << curr_msg_->busmsg << std::endl;
            // std::cout << "Traffic count " << stats_->interconstats.traffic << std::endl;
            curr_msg_.reset();

        }

        turn_ = (turn_ + 1) % num_proc_;
    }

    SnoopBus::SnoopBus(size_t num_proc, Caches *caches, Stats *stats) : num_proc_(num_proc), caches_(caches), stats_(stats)
    {
        curr_msg_ = std::nullopt;
        turn_ = 0;
    }

    void SnoopBus::requestFromCache(BusMsg busmsg)
    {
        // record interconnect traffic
        stats_->interconstats.traffic++;

        // received request from cache.
        // can be rd/wr/upg
        assert(busmsg.src_proc_ == turn_);
        assert(busmsg.type_ == BUSREAD || busmsg.type_ == BUSWRITE || busmsg.type_ == BUSUPGRADE);
        assert(!curr_msg_);
        curr_msg_ = CurrMsg{.busmsg = busmsg, .state = PROCESSING};
    }

    void SnoopBus::replyFromCache(BusMsg busmsg)
    {
        // record interconnect traffic
        stats_->interconstats.traffic++;

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

    bool SnoopBus::isCacheTurn(size_t proc)
    {
        return proc == turn_;
    }
}