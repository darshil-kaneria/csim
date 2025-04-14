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
            os << "COMPULSORY:\t" << cachestat.compulsory_misses << "\n";
            os << "CAPACITY:\t" << cachestat.capacity_evicts << "\n";
            os << "CONFLICT:\t" << cachestat.conflict_evicts << "\n";
            os << "COHERENCE:\t" << cachestat.coherence_evicts << "\n";
            os << "----------------------------------\n";
        }

        os << "INTERCONNECT" << "\n";
        os << "TRAFFIC:\t" << stats.interconstats.traffic << "\n";

        return os;
    }
    Stats::Stats(size_t num_procs) : num_procs_(num_procs)
    {
        cachestats = std::vector<CacheStats>(num_procs, CacheStats{
                                                            .capacity_evicts = 0,
                                                            .coherence_evicts = 0,
                                                            .compulsory_misses = 0,
                                                            .conflict_evicts = 0,
                                                            .hits = 0,
                                                            .misses = 0});
        interconstats = InterconnectStats{.traffic = 0};
    }
}