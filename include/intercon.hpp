#pragma once

#include "busmsg.hpp"
#include <optional>
#include <vector>

namespace csim
{
    class SnoopIntercon
    {
    public:
        bool tick();
        void SendBusMessage(BusMsg bus_msg);

    private:
        int num_proc_;
        std::optional<BusMsg> curr_msg_;
        std::vector<std::optional<BusMsg>> queued_msgs_;
        uint8_t last_proc_;
        int delay_;
    };

}