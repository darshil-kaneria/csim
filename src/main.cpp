#include "trace.hpp"
#include <cstdint>
#include <string>
#include "cpu.hpp"
#include "bus.hpp"
#include "cache.hpp"
#include <iostream>
#include <fstream>
#include "globals.hpp"

using namespace csim;

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    size_t num_procs = 4;
    std::string directory = "traces";

    std::cout << "No of Processors: " << num_procs << std::endl;
    CoherenceProtocol coherproto = static_cast<CoherenceProtocol>(0);
    TraceReader tr(directory, num_procs);

    CPUS cpus(&tr, num_procs, nullptr);
    Caches caches(num_procs, nullptr, &cpus, coherproto);
    SnoopBus bus(num_procs, &caches);

    cpus.setCaches(&caches);
    caches.setBus(&bus);

    bool running = false;
    do
    {
        running = cpus.cycle();
    } while (running);

    std::cout << "Cycle " << cycles << std::endl;
}
