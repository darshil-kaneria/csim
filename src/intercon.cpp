#include "intercon.hpp"
#include <cassert>
#include "cache.hpp"

namespace csim
{
    uint8_t CACHE_DELAY_TIME = 10;
    uint8_t MEMORY_DELAY_TIME = 90;

    bool SnoopIntercon::tick()
    {
        if (delay_ == 0)
        {
            // can process the next message.
            for (uint8_t i = 1; i <= num_proc_; i++)
            {
                uint8_t proc = (i + last_proc_) % num_proc_;

                if (queued_msgs_[proc])
                {
                    BusMsg &bus_msg = queued_msgs_[proc].value();
                    curr_msg_ = bus_msg;
                    delay_ = CACHE_DELAY_TIME;

                    queued_msgs_[proc] = std::nullopt;

                    last_proc_ = proc;
                    break;
                }
            }
            return true;
        }

        // A message is on the bus.
        assert(curr_msg_ != std::nullopt);

        delay_--;

        if (delay_ > 0)
        {
            // nothing to do for now
            return true;
        }

        // delay == 0, some action should occur
        if (curr_msg_->state == CACHE_DELAY)
        {
            // Cache delay is over. Simulate broadcasting message out to caches and memory.
            delay_ = MEMORY_DELAY_TIME;
            curr_msg_->state = REQUESTED_DATA;

            // send snoop message to caches
            curr_msg_->cache->requestFromBus(curr_msg_.value());

            // caches might provide data
            if (curr_msg_->state == CACHE_PROVIDED_DATA)
            {
                // TODO send notification that data has been provided
                curr_msg_->cache->replyFromBus(curr_msg_.value());
                curr_msg_ = std::nullopt;
                delay_ = 0;
            }
            return true;
        }

        if (curr_msg_->state == REQUESTED_DATA)
        {
            // Memory delay is over. No cache provided data. Memory should provide data.

            curr_msg_->state = MEMORY_PROVIDED_DATA;

            // TODO send notification that data has been provided
            curr_msg_->cache->replyFromBus(curr_msg_.value());

            curr_msg_ = std::nullopt;
            delay_ = 0;
        }
        return true;
    }

    void SnoopIntercon::requestFromCache(BusMsg bus_msg)
    {
        // a bus request is received on the interconnect from a cache.
        // ideally should broacast immediately but we simulate time by waiting for CACHE DELAY

        assert(bus_msg.type == BUSREAD || bus_msg.type == BUSWRITE);

        if (!curr_msg_)
        {
            // No message is being processed on the bus
            curr_msg_ = bus_msg;
            curr_msg_->state = CACHE_DELAY;
            delay_ = CACHE_DELAY_TIME;
            return;
        }

        // There is a message on the bus, put on processors slot, first ensure processor's slot is empty.
        assert(!queued_msgs_[bus_msg.memreq.proc_]);
        queued_msgs_[bus_msg.memreq.proc_] = bus_msg;
    }

    void SnoopIntercon::replyFromCache(BusMsg bus_msg)
    {
        // a bus reply is received on the interconnect from a processor.
        // tell all the caches.

        assert(bus_msg.type == BUSDATA || bus_msg.type == BUSSHARED);
        assert(curr_msg_.has_value());
        assert(curr_msg_->memreq == bus_msg.memreq);

        // send message to all caches
        bus_msg.state = CACHE_PROVIDED_DATA;
        bus_msg.cache->replyFromBus(bus_msg);
    }

    SnoopIntercon::SnoopIntercon(int num_proc) : num_proc_(num_proc)
    {
        curr_msg_ = std::nullopt;
        queued_msgs_ = std::vector<std::optional<BusMsg>>(num_proc_, std::nullopt);
        last_proc_ = num_proc_;
        delay_ = 0;
    }
}