#pragma once

#include "busmsg.hpp"
#include <optional>
#include <vector>

namespace csim
{
    class SnoopBus
    {
    public:
        bool tick();
        SnoopBus(size_t num_proc = 8);

    private:
        size_t num_proc_;
        std::priority_queue<BusMsg> inputq;
        
    };

}