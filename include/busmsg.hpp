#pragma once

#include "memreq.hpp"

namespace csim
{
    class Caches;

    enum BusMsgType
    {
        BUSREAD,
        BUSWRITE,
    };

    enum BusMsgState
    {
        NONE,
        QUEUED,
        CACHE_DELAY,
        MEMORY_DELAY,
        CACHE_PROVIDED_DATA,
        MEMORY_PROVIDED_DATA,
    };

    struct BusMsg
    {
        BusMsgType type;
        BusMsgState state;
        MemReq memreq; // The memory request that triggered this bus packet originally (if any)
        Caches *cache;
    };

    

    

}