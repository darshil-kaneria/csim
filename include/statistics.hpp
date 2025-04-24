#pragma once

#include <vector>
#include "stddef.h"
#include <iostream>

namespace csim
{

    struct CacheStats
    {
        size_t hits;
        size_t misses;
        size_t evictions;
    };

    struct InterconnectStats
    {
        size_t traffic;
        size_t cache_control_traffic;
        size_t cache_data_traffic;
        size_t mem_data_traffic;
    };

    struct Stats
    {
        std::size_t num_procs_;
        std::vector<CacheStats> cachestats;
        InterconnectStats interconstats;
        Stats(size_t num_procs);
    };

    std::ostream &operator<<(std::ostream &os, const Stats &stats);
}