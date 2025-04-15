#pragma once

#include "busmsg.hpp"
#include <optional>
#include <vector>
#include <unordered_set>
#include "statistics.hpp"

namespace csim
{
    enum State
    {
        PROCESSING,
        CACHEDATA,
        CACHESHARED,
    };

    struct CurrMsg
    {
        BusMsg busmsg;
        State state;
    };

    class SnoopBus
    {
    public:
        SnoopBus(size_t num_proc, SnoopCaches *caches, Stats *stats);
        void cycle();
        void requestFromCache(BusMsg busmsg);
        void replyFromCache(BusMsg busmsg);
        void setCaches(SnoopCaches *caches);
        bool isCacheTurn(size_t proc);

    private:
        size_t num_proc_;
        size_t turn_;
        std::optional<CurrMsg> curr_msg_;
        SnoopCaches *caches_;
        Stats *stats_;
    };

}