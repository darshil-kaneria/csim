#include "statistics.hpp"
#include <iostream>

namespace csim
{
    std::ostream &operator<<(std::ostream &os, const Stats &stats)
    {
        for (size_t proc = 0; proc < stats.num_procs_; proc++)
        {
            const CacheStats &cachestat = stats.cachestats[proc];
            os << "PROCESSOR " << proc << "\n";
            os << "HITS:\t\t" << cachestat.hits << "\n";
            os << "MISSES:\t\t" << cachestat.misses << "\n";
            os << "COHERENCE:\t" << cachestat.evictions << "\n";
            os << "----------------------------------\n";
        }

        size_t hits = 0;
        size_t misses = 0;
        size_t evictions = 0;
        for(size_t proc = 0; proc < stats.num_procs_; proc++) {
            hits += stats.cachestats[proc].hits;
            misses += stats.cachestats[proc].misses;
            evictions += stats.cachestats[proc].evictions;
        }

        os << "TOTAL HITS: \t" << hits << "\n";
        os << "TOTAL MISSES: \t" << misses << "\n";
        os << "TOTAL EVICTIONS: \t" << evictions << "\n\n";

        os << "INTERCONNECT" << "\n";
        os << "TRAFFIC:\t" << stats.interconstats.traffic << "\n";
        os << "CACHE CONTROL TRAFFIC:\t" << stats.interconstats.cache_control_traffic << "\n";
        os << "CACHE DATA TRAFFIC:\t" << stats.interconstats.cache_data_traffic << "\n";
        os << "MEMORY DATA TRAFFIC:\t" << stats.interconstats.mem_data_traffic << "\n";
        return os;
    }

    Stats::Stats(size_t num_procs) : num_procs_(num_procs)
    {
        cachestats = std::vector<CacheStats>(num_procs, CacheStats{
                                                            .evictions = 0,
                                                            .hits = 0,
                                                            .misses = 0});
        interconstats = InterconnectStats{.traffic = 0};
    }
}