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
        std::priority_queue<BusMsg> inputqbus;
        std::priority_queue<CPUMsg> inputqcpu;
    };

    class Caches
    {
    public:
        void tick();
        Caches(size_t num_procs = 8, SnoopIntercon *intercon = nullptr, CoherenceProtocol coherproto = MI);

    private:
        size_t num_procs_;
        SnoopBus *snoopbus_;
        CPUS *cpu;
        std::vector<Cache> caches_;
        CoherenceProtocol coherproto_;
    };

}