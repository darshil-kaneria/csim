#include <unordered_map>
#include <unordered_set>
#include "dirmsg.hpp"

namespace csim
{

    struct DirEntry
    {
        size_t address;
        std::optional<size_t> owner;
        std::unordered_set<size_t> sharers;
    };

    class Directory
    {
    public:
        Directory(size_t num_procs);
        void requestFromCache(DirMsg dirreq);

    private:
        size_t num_procs_;
        std::unordered_map<size_t, DirEntry> directory_;
    };
}