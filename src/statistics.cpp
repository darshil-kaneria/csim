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