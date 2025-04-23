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
            assert(curr_msg_->state == BusState::PROCESSING);

            std::cout << "Processing " << curr_msg_->busmsg << std::endl;

            BusMsg &busreq = curr_msg_->busmsg;
            assert(busreq.type_ == BusMsgType::READ || busreq.type_ == BusMsgType::WRITE || busreq.type_ == BusMsgType::UPGRADE);

            // send to all processors except source
            size_t src = busreq.src_proc_;

            for (size_t proc = 0; proc < num_proc_; proc++)
            {
                if (src != proc)
                {
                    // record interconnect traffic
                    stats_->interconstats.traffic++;
                    stats_->interconstats.cache_control_traffic++;

                    BusMsg msg = busreq;
                    msg.dst_proc_ = proc;
                    caches_->requestFromBus(msg);
                }
            }

            // caches will change state if necessary. Now check state.
            BusMsg busresp;
            busresp = curr_msg_->busmsg;
            busresp.dst_proc_ = src;

            if (curr_msg_->state == BusState::CACHEDATA)
            {
                // cache flushed data
                busresp.type_ = BusMsgType::DATA;

                stats_->interconstats.traffic++;
                stats_->interconstats.cache_data_traffic++;
            }
            else if (curr_msg_->state == BusState::CACHESHARED)
            {
                // cache provided data in shared mode
                busresp.type_ = BusMsgType::SHARED;

                stats_->interconstats.traffic++;
                stats_->interconstats.cache_data_traffic++;
            }
            else if (curr_msg_->state == BusState::PROCESSING)
            {
                // TODO maybe handle edge case for upgrades, not necessary for now.
                // cache didn't provide data, memory will
                busresp.type_ = BusMsgType::MEMDATA;

                // simulate traffic for memory providing data.
                stats_->interconstats.traffic+=2;
                stats_->interconstats.mem_data_traffic+=2;
            }

            caches_->replyFromBus(busresp);

            std::cout << "Processed " << curr_msg_->busmsg << std::endl;
            // std::cout << "Traffic count " << stats_->interconstats.traffic << std::endl;
            curr_msg_.reset();
        }

        turn_ = (turn_ + 1) % num_proc_;
    }

    SnoopBus::SnoopBus(size_t num_proc, SnoopCaches *caches, Stats *stats) : num_proc_(num_proc), caches_(caches), stats_(stats)
    {
        curr_msg_ = std::nullopt;
        turn_ = 0;
    }

    void SnoopBus::requestFromCache(BusMsg busmsg)
    {
        // record interconnect traffic
        stats_->interconstats.traffic++;
        stats_->interconstats.cache_control_traffic++;

        // received request from cache.
        // can be rd/wr/upg
        assert(busmsg.src_proc_ == turn_);
        assert(busmsg.type_ == BusMsgType::READ || busmsg.type_ == BusMsgType::WRITE || busmsg.type_ == BusMsgType::UPGRADE);
        assert(!curr_msg_);
        curr_msg_ = CurrMsg{.busmsg = busmsg, .state = BusState::PROCESSING};
    }

    void SnoopBus::replyFromCache(BusMsg busmsg)
    {
        // record interconnect traffic
        stats_->interconstats.traffic++;

        // received reply from cache.
        // can be data/shared
        assert(curr_msg_);
        assert(busmsg.type_ == BusMsgType::DATA || busmsg.type_ == BusMsgType::SHARED);

        if (busmsg.type_ == BusMsgType::DATA)
        {
            stats_->interconstats.cache_data_traffic++;
            curr_msg_->state = BusState::CACHEDATA;
        }
        else
        {
            stats_->interconstats.cache_data_traffic++;
            curr_msg_->state = BusState::CACHESHARED;
        }
    }

    void SnoopBus::setCaches(SnoopCaches *caches)
    {
        caches_ = caches;
    }

    bool SnoopBus::isCacheTurn(size_t proc)
    {
        return proc == turn_;
    }
}