#pragma once

#include <optional>
#include "memreq.hpp"
#include <cstdint>
#include <vector>
#include "cpu.hpp"
#include <queue>
#include "busmsg.hpp"

namespace csim
{
    class SnoopIntercon;
    enum CoherenceStates
    {
        MODIFIED,
        INVALID,
        SHARED,
        EXCLUSIVE,
        // TODO: Add intermediary states
    };

    enum CoherenceProtocol
    {
        MI,
        MSI,
        MESI
    };

    struct Line
    {
        CoherenceStates coherstate;
    };
    // This is the cache class
    struct Cache
    {
        std::unordered_map<uint64_t, Line> lines;
    };

    class Caches
    {
    public:
        void tick();
        void requestFromProcessor(MemReq memReq);
        void requestFromBus(BusMsg bus_msg);
        void replyFromBus(BusMsg bus_msg);
        Caches(int num_procs = 8, SnoopIntercon *intercon = nullptr, CoherenceProtocol coherproto = MI);

    private:
        int num_procs_;
        bool isReqAHit(MemReq &memReq);
        std::vector<std::optional<MemReq>> pending_requests_;
        SnoopIntercon *intercon_;
        std::vector<Cache> caches_;
        CoherenceProtocol coherproto_;
    };

}