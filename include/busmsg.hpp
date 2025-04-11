#pragma once

#include "memreq.hpp"

namespace csim
{
    class Caches;

    enum BusMsgType
    {
        BUSREAD,
        BUSWRITE,
        BUSDATA,
        BUSSHARED
    };

    enum BusMsgState
    {
        NONE,
        QUEUED,
        CACHE_DELAY,
        REQUESTED_DATA,
        CACHE_PROVIDED_DATA,
        MEMORY_PROVIDED_DATA,
    };

    struct BusMsg
    {
        BusMsgType type;
        BusMsgState state;
        MemReq memreq; // The memory request that triggered this bus packet originally (if any)
        Caches *cache;
        uint8_t src_proc_;
        uint8_t dst_proc_;
    };

    std::ostream &operator<<(std::ostream &os, const BusMsgState &busmsgstate);
    std::ostream &operator<<(std::ostream &os, const BusMsgType &busmsgtype);
    std::ostream &operator<<(std::ostream &os, const BusMsg &busmsg);

}