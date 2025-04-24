#include "trace.hpp"
#include <cstdint>
#include <string>
#include "cpu.hpp"
#include "bus.hpp"
#include "cache.hpp"
#include <iostream>
#include <fstream>
#include "directory.hpp"
#include "globals.hpp"
#include "argparser.hpp"

using namespace csim;

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Config config;
    parseArgs(argc, argv, config);
    printConfig(config);

    size_t num_procs = config.num_procs;
    std::string directory = config.directory;
    CoherenceType cohertype = config.cohertype;
    CoherenceProtocol coherproto = config.coherproto;
    size_t cache_line_size = config.cache_line_size;
    size_t cache_size = config.cache_size;
    bool dir_opt = config.diropt;

    std::cout << "No of Processors: " << num_procs << std::endl;
    std::cout << "Directory: " << directory << std::endl;
    std::cout << "Coherence Type: " << cohertype << std::endl;
    std::cout << "Coherence Protocol: " << coherproto << std::endl;
    std::cout << "Cache Line Size: " << cache_line_size << std::endl;
    std::cout << "Cache Size: " << cache_size << std::endl;
    std::cout << "Directory Optimization " << ((dir_opt == true || dir_opt == 1) ? "True" : "False") << std::endl;


    TraceReader tr(directory, num_procs, cache_line_size);
    Stats stats(num_procs);
    CPUS cpus(&tr, num_procs, nullptr);

    if (cohertype == CoherenceType::SNOOP)
    {
        SnoopCaches snoopcaches(num_procs, nullptr, &cpus, coherproto, &stats);
        SnoopBus bus(num_procs, &snoopcaches, &stats);
        snoopcaches.setBus(&bus);
        cpus.setCaches(&snoopcaches);

        bool running = false;
        do
        {
            running = cpus.cycle();
        } while (running);
    }
    else
    {
        DirectoryCaches dircaches(num_procs, nullptr, &cpus, &stats);
        Directory dir(num_procs, &dircaches, &stats);
        dircaches.setDirectory(&dir);
        cpus.setCaches(&dircaches);

        bool running = false;
        do
        {
            running = cpus.cycle();
        } while (running);
    }

    std::cout << stats << std::endl;
}
