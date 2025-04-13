#pragma once

#include "busmsg.hpp"
#include <optional>
#include <vector>
#include <unordered_set>

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
        SnoopBus(size_t num_proc, Caches *caches);
        void cycle();
        void requestFromCache(BusMsg busmsg);
        void replyFromCache(BusMsg busmsg);
        void setCaches(Caches *caches);
        bool isCacheTurn(size_t proc);

    private:
        size_t num_proc_;
        size_t turn_;
        std::optional<CurrMsg> curr_msg_;
        Caches *caches_;
    };

}