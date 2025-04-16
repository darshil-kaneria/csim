#pragma once

#include "busmsg.hpp"
#include <cstdint>
#include <vector>
#include "cpu.hpp"
#include <queue>
#include "cpumsg.hpp"
#include "dirmsg.hpp"
#include "statistics.hpp"
#include <list>

namespace csim
{
    class SnoopBus;
    class Directory;

    enum class CoherenceState
    {
        MODIFIED,
        INVALID,
        SHARED,
        EXCLUSIVE,
        OWNED,
        FORWARDER
    };

    std::ostream& operator<<(std::ostream& os, const CoherenceState& state);

    enum class CoherenceProtocol
    {
        MI,
        MSI,
        MESI,
        MOESI,
        MESIF
    };
    std::ostream& operator<<(std::ostream& os, const CoherenceProtocol& state);

    struct Line
    {
        size_t tag;
        bool valid;
        CoherenceState coherstate;
    };

    struct LRUSet
    {
        size_t num_lines;
        std::list<Line> linelist;
        std::unordered_map<size_t, std::list<Line>::iterator> linemap;
    };

    // This is the cache class
    struct Cache
    {
        // TODO: maybe make cache better.
        size_t cachesize;
        size_t blocksize;
        size_t associativity;
        size_t num_sets;
        std::unordered_map<size_t, LRUSet> sets;

        std::unordered_map<size_t, CoherenceState> lines;
        std::optional<CPUMsg> pending_cpu_req;
    };

    class SnoopCaches
    {
    public:
        SnoopCaches(size_t num_procs, SnoopBus *snoopbus, CPUS *cpus, CoherenceProtocol coherproto, Stats *stats);
        void cycle();
        void requestFromProcessor(CPUMsg cpumsg);
        void requestFromBus(BusMsg busmsg);
        void replyFromBus(BusMsg busmsg);
        void setBus(SnoopBus *snoopbus);
        void setCPUs(CPUS *cpus);

    private:
        size_t num_procs_;
        SnoopBus *snoopbus_;
        CPUS *cpus_;
        std::vector<Cache> caches_;
        CoherenceProtocol coherproto_;
        Stats *stats_;

        bool isAHit(CPUMsg &cpureq, size_t proc);
        CoherenceState getCoherenceState(size_t address, size_t proc);
        void setCoherenceState(size_t address, CoherenceState newstate, size_t proc);

        std::optional<BusMsg> requestFromBusMI(BusMsg &busreq, size_t proc);
        CPUMsg replyFromBusMI(BusMsg &busresp, size_t proc);
        bool isAHitMI(CPUMsg &cpureq, size_t proc);

        std::optional<BusMsg> requestFromBusMSI(BusMsg &busreq, size_t proc);
        CPUMsg replyFromBusMSI(BusMsg &busresp, size_t proc);
        bool isAHitMSI(CPUMsg &cpureq, size_t proc);

        std::optional<BusMsg> requestFromBusMESI(BusMsg &busreq, size_t proc);
        CPUMsg replyFromBusMESI(BusMsg &busresp, size_t proc);
        bool isAHitMESI(CPUMsg &cpureq, size_t proc);

        std::optional<BusMsg> requestFromBusMOESI(BusMsg &busreq, size_t proc);
        CPUMsg replyFromBusMOESI(BusMsg &busresp, size_t proc);
        bool isAHitMOESI(CPUMsg &cpureq, size_t proc);

        std::optional<BusMsg> requestFromBusMESIF(BusMsg &busreq, size_t proc);
        CPUMsg replyFromBusMESIF(BusMsg &busresp, size_t proc);
        bool isAHitMESIF(CPUMsg &cpureq, size_t proc);
    };

    class DirectoryCaches
    {
    public:
        DirectoryCaches(size_t num_procs, Directory *directory, CPUS *cpus, Stats *stats);
        void cycle();
        void requestFromProcessor(CPUMsg cpureq);
        void requestFromDirectory(DirMsg dirreq);
        void replyFromDirectory(DirMsg dirresp);
        void setDirectory(Directory *directory);
        void setCPUs(CPUS *cpus);

    private:
        size_t num_procs_;
        CPUS *cpus_;
        Directory *directory_;
        std::vector<Cache> caches_;
        Stats *stats_;

        bool isAHit(CPUMsg &cpureq, size_t proc);
        CoherenceState getCoherenceState(size_t address, size_t proc);
        void setCoherenceState(size_t address, CoherenceState newstate, size_t proc);
    };

}