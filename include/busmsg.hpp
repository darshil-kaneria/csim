#pragma once

#include "cpumsg.hpp"

namespace csim
{
    class Caches;

    const size_t BUSDELAY = 10;
    const size_t MEMDELAY = 100;

    const size_t BROADCAST = 1000;

    enum BusMsgType
    {
        BUSREAD,
        BUSWRITE,
        BUSUPGRADE,
        BUSDATA,
        BUSSHARED,
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