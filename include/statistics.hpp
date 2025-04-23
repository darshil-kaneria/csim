#pragma once

#include <vector>

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
        size_t data_traffic;
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