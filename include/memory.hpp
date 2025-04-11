#pragma once

#include "busmsg.hpp"

namespace csim
{

    class SnoopBus;

    class Memory
    {
    public:
        void tick();
        void enqueueMsgFromBus(BusMsg busmsg);
        Memory(SnoopBus* bus);
        void setBus(SnoopBus* bus);

    private:
        std::priority_queue<BusMsg, std::vector<BusMsg>, BusMsgComparator> inputq_;
        SnoopBus *bus_;
    };
} 