#include "trace.hpp"
#include <cstdint>
#include <string>
#include "cpu.hpp"
#include "bus.hpp"
#include "cache.hpp"
#include <iostream>

using namespace csim;

int main(int argc, char *argv[])
{

    size_t num_procs = 8;
    std::string directory = "";

    std::cout << "No of Processors: " << num_procs << std::endl;
    CoherenceProtocol coherproto = static_cast<CoherenceProtocol>(0);
    TraceReader tr{.num_procs_ = num_procs, .directory_ = directory};

    CPUS cpus(&tr, num_procs, nullptr);
    Caches caches(num_procs, nullptr, &cpus, coherproto);
    SnoopBus bus(num_procs, &caches);

    cpus.setCaches(&caches);
    caches.setBus(&bus);

    bool running = false;
    do
    {
        running = cpus.tick();
    } while (running);
}
