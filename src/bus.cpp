#include "bus.hpp"
#include "globals.hpp"
#include "cache.hpp"
#include "memory.hpp"

namespace csim
{
    void SnoopBus::tick()
    {
        // process messages on the bus
        while (!inputq_.empty() && inputq_.top().proc_cycle_ <= cycle)
        {
            BusMsg busmsg = inputq_.top();
            size_t src = busmsg.src_proc_;

            if (src == MEMORY)
            {
                // check that data has not been provided

                // broadcast to all caches
                for (int proc = 0; proc < num_proc_; proc++)
                {
                    caches_->enqueueMsgFromBus(busmsg, proc);
                }
            }
            else
            {

                // src is cache tell all others except cache
                
                for (int proc = 0; proc < num_proc_; proc++)
                {
                    if (proc == src)
                    {
                        continue;
                    }
                    caches_->enqueueMsgFromBus(busmsg, proc);
                }

                // tell memory if response has not been previously provided
                busmsg.proc_cycle_ += MEMDELAY;
                memory_->enqueueMsgFromBus(busmsg);
            }
            inputq_.pop();
        }
        memory_->tick();
    }
    SnoopBus::SnoopBus(size_t num_proc, Caches *caches, Memory *memory) : num_proc_(num_proc), caches_(caches), memory_(memory)
    {
        inputq_ = std::priority_queue<BusMsg, std::vector<BusMsg>, BusMsgComparator>();
    }
    void SnoopBus::enqueMsg(BusMsg busmsg)
    {
        inputq_.push(busmsg);
    }
    void SnoopBus::setCaches(Caches *caches)
    {
        caches_ = caches;
    }
    void SnoopBus::setMemory(Memory *memory)
    {
        memory_ = memory;
    }
}