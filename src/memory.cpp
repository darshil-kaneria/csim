#include "memory.hpp"
#include "globals.hpp"
#include "bus.hpp"

namespace csim
{
    void Memory::tick()
    {
        // process received messages
        while (!inputq_.empty() && inputq_.top().proc_cycle_ <= cycle)
        {
            BusMsg busmsg = inputq_.top();

            // if message is a busRd/busWrite provide data.
            if (busmsg.type_ == BusMsgType::BUSREAD || busmsg.type_ == BusMsgType::BUSWRITE)
            {
                busmsg.type_ = BusMsgType::BUSDATA;
                busmsg.proc_cycle_ = cycle + MEMDELAY;
                bus_->enqueMsg(busmsg);
            }

            inputq_.pop();
        }
    }
    void Memory::enqueueMsgFromBus(BusMsg busmsg)
    {
        inputq_.push(busmsg);
    }
    Memory::Memory(SnoopBus *bus) : bus_(bus)
    {
    }
    void Memory::setBus(SnoopBus *bus)
    {
        bus_ = bus;
    }
}
