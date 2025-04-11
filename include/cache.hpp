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
        std::priority_queue<BusMsg, std::vector<BusMsg>, BusMsgComparator> inputqbus;
        std::priority_queue<CPUMsg, std::vector<CPUMsg>, CPUMsgComparator> inputqcpu;
        std::optional<BusMsg> pendingmsg;

        bool isAHit(CPUMsg cpureq);
    };

    class Caches
    {
    public:
        void tick();
        void enqueueMsgFromProc(CPUMsg &cpumsg, size_t proc);
        void enqueueMsgFromBus(BusMsg &busmsg, size_t proc);
        Caches(size_t num_procs, SnoopBus *snoopbus, CPUS *cpus, CoherenceProtocol coherproto);
        void setBus(SnoopBus* snoopbus);
        void setCPUs(CPUS* cpus);

    private:
        size_t num_procs_;
        SnoopBus *snoopbus_;
        CPUS *cpus_;
        std::vector<Cache> caches_;
        CoherenceProtocol coherproto_;
    };

}