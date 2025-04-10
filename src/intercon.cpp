#include "intercon.hpp"
#include <cassert>
#include "cache.hpp"

namespace csim
{
    uint8_t CACHE_DELAY_TIME = 10;
    uint8_t MEMORY_DELAY_TIME = 90;

    bool csim::SnoopIntercon::tick()
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

            // send snoop message to caches
            curr_msg_->cache->trafficFromBus(curr_msg_.value());

            // caches might provide data
            if (curr_msg_->state == CACHE_PROVIDED_DATA)
            {
                // TODO send notification that data has been provided
                curr_msg_->cache->dataProvidedFromBus(curr_msg_.value());
                curr_msg_ = std::nullopt;
                delay_ = 0;
            }
            return true;
        }

        if (curr_msg_->state == MEMORY_DELAY)
        {

            // Memory delay is over. No cache provided data. Memory should provide data.

            curr_msg_->state = MEMORY_PROVIDED_DATA;
            // TOD send notification that data has been provided
            curr_msg_->cache->dataProvidedFromBus(curr_msg_.value());

            curr_msg_ = std::nullopt;
            delay_ = 0;
        }
        return false;
    }

    void csim::SnoopIntercon::SendBusMessage(BusMsg bus_msg)
    {
    }
}