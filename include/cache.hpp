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
        FORWARDER,
        OWNED,
        // TODO: Add intermediary states
    };

    // This will contain the valid bit, dirty bit, tag, and the data.
    class Line
    {
    public:
        Line() = default;
        ~Line() = default;

        void setLine(std::vector<uint8_t> data);
        void setDirty(uint32_t tag);
        void setValid(uint32_t tag, bool is_valid);

    private:
        uint32_t tag_;
        bool is_valid_;
        bool is_dirty_;
        CoherenceStates coher_state_;
    };

    // This holds the lines per set (a simple vec)
    class Set
    {
    public:
        Set() = default;
        ~Set() = default;

        std::vector<Line> &getLines();
        Line &setLine(uint32_t tag);
        Line &getLine(uint32_t tag);

    private:
        std::vector<Line> lines_;
    };

    // This is the cache class
    class Cache
    {
    public:
        Cache() = default;
        Cache(uint32_t set_assoc, uint32_t block_size, uint32_t tag_size, uint32_t addr_len);
        ~Cache();

    private:
        uint32_t set_assoc_;
        uint32_t block_size_;
        uint32_t tag_size_;
        uint32_t addr_len_;
        uint32_t size;
    };

    class Caches
    {
    public:
        void tick();
        void requestFromProcessor(MemReq memReq);
        void requestFromBus(BusMsg bus_msg);
        void replyFromBus(BusMsg bus_msg);
        Caches(int num_procs = 8, SnoopIntercon *intercon = nullptr);

    private:
        int num_procs_;
        bool isReqAHit(MemReq &memReq);
        std::vector<std::optional<MemReq>> pending_requests_;
        SnoopIntercon *intercon_;
        std::vector<Cache> caches_;
    };

}