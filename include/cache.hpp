#pragma once

#include "busmsg.hpp"
#include <cstdint>
#include <vector>
#include "cpu.hpp"
#include <queue>
#include "cpumsg.hpp"

namespace csim
{
    class SnoopBus;
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
        std::unordered_map<size_t, Line> lines;
        std::optional<BusMsg> pending_bus_msg;
        CoherenceProtocol coherproto_;
        bool isAHit(CPUMsg &cpureq);
    };

    class Caches
    {
    public:
        void tick();
        void requestFromProcessor(CPUMsg cpumsg);
        void requestFromBus(BusMsg busmsg);
        void replyFromBus(BusMsg busmsg);
        Caches(size_t num_procs, SnoopBus *snoopbus, CPUS *cpus, CoherenceProtocol coherproto);
        void setBus(SnoopBus *snoopbus);
        void setCPUs(CPUS *cpus);

    private:
        size_t num_procs_;
        SnoopBus *snoopbus_;
        CPUS *cpus_;
        std::vector<Cache> caches_;
        CoherenceProtocol coherproto_;
    };

}