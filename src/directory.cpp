#include "directory.hpp"

namespace csim
{
    Directory::Directory(size_t num_procs) : num_procs_(num_procs)
    {
        directory_ = std::unordered_map<size_t, DirEntry>();
    }

    void Directory::requestFromCache(DirMsg dirreq)
    {
    }
}
