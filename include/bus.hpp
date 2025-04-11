#pragma once

#include "busmsg.hpp"
#include <optional>
#include <vector>
#include <unordered_set>

namespace csim
{
    class Memory;

    class SnoopBus
    {
    public:
        void tick();
        SnoopBus(size_t num_proc, Caches* caches, Memory* memory);
        void enqueMsg(BusMsg busmsg);
        void setCaches(Caches* caches);
        void setMemory(Memory* memory);

    private:
        size_t num_proc_;
        std::priority_queue<BusMsg, std::vector<BusMsg>, BusMsgComparator> inputq_;
        Caches* caches_;
        Memory* memory_;
    };

}