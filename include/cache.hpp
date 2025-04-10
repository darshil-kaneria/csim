#pragma once

#include <cstdint>
#include <vector>
#include <cpu.hpp>
#include <memreq.hpp>

namespace csim
{
    enum CoherenceStates {
        MODIFIED,
        INVALID,
        SHARED,
        EXCLUSIVE,
        FORWARDER,
        OWNED,
        // TODO: Add intermediary states
    };

    class Cache
    {
    public:
        bool tick();
        void requestFromProcessor(MemoryRequest memReq);

    private:
        int num_caches;
        std::vector<Cache> caches;
        
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
    // class Cache
    // {
    // public:
    //     Cache() = default;
    //     Cache(uint32_t set_assoc, uint32_t block_size, uint32_t tag_size, uint32_t addr_len);
    //     ~Cache();

    // private:
    //     uint32_t set_assoc_;
    //     uint32_t block_size_;
    //     uint32_t tag_size_;
    //     uint32_t addr_len_;
    //     uint32_t size;
    // };
}