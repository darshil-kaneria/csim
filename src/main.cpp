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

using namespace csim;

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    size_t num_procs = 4;
    std::string directory = "traces";

    CoherenceType cohertype = static_cast<CoherenceType>(0);
    CoherenceProtocol coherproto = static_cast<CoherenceProtocol>(4);

    std::cout << "No of Processors: " << num_procs << std::endl;
    std::cout << "Coherence Type: " << cohertype << std::endl;
    std::cout << "Coherence Protocol: " << coherproto << std::endl;

    TraceReader tr(directory, num_procs);

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
