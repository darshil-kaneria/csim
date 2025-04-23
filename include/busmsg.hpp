#pragma once

#include "cpumsg.hpp"

namespace csim
{
    class SnoopCaches;


    const size_t BROADCAST = 1000;

    enum class BusMsgType
    {
        READ, 
        WRITE,
        UPGRADE,
        DATA,
        SHARED,
        MEMDATA
    };

    struct BusMsg
    {
        size_t proc_cycle_;
        BusMsgType type_;
        CPUMsg cpureq_; // The cpu request that triggered this bus packet originally (if any)
        size_t src_proc_;
        size_t dst_proc_;
    };

    std::ostream &operator<<(std::ostream &os, const BusMsgType &busmsgtype);
    std::ostream &operator<<(std::ostream &os, const BusMsg &busmsg);
}