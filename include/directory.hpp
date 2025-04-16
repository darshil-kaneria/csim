#pragma once

#include <unordered_map>
#include <unordered_set>
#include "dirmsg.hpp"
#include "statistics.hpp"

namespace csim
{

    class DirectoryCaches;

    enum class DirState
    {
        UNCACHED,
        SHARED,
        EXCLUSIVE
    };

    struct DirEntry
    {
        DirState state;
        size_t address;
        std::optional<size_t> owner;
        std::unordered_set<size_t> sharers;
    };

    class Directory
    {
    public:
        Directory(size_t num_procs, DirectoryCaches *caches, Stats *stats);
        void requestFromCache(DirMsg dirreq);

    private:
        size_t num_procs_;
        std::unordered_map<size_t, DirEntry> directory_;
        DirectoryCaches *caches_;
        Stats *stats_;
        DirState GetState(size_t address);
        void SetState(size_t address, size_t proc, DirState newstate);
    };
}