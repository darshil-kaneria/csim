#pragma once

#include "busmsg.hpp"
#include <optional>
#include <vector>
#include <unordered_set>

namespace csim
{
    class Memory;

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
        void tick();
        SnoopBus(size_t num_proc, Caches *caches);
        void requestFromCache(BusMsg busmsg);
        void replyFromCache(BusMsg busmsg);
        void setCaches(Caches *caches);

    private:
        size_t num_proc_;
        std::queue<BusMsg> inputq_;
        std::optional<CurrMsg> curr_msg_;
        Caches *caches_;
    };

}