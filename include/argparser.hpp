#pragma once

#include <string>
#include <vector>
#include "cache.hpp"

namespace csim
{
    struct Config
    {
        size_t num_procs;
        std::string directory;
        CoherenceType cohertype;
        CoherenceProtocol coherproto;
        size_t cache_line_size = 64;
        size_t cache_size = 8192;
        bool diropt;
    };

    std::vector<std::string> split(const std::string &s, char delimiter);
    void parseArgs(int argc, char *argv[], Config &config);
    void printConfig(const Config &config);
}