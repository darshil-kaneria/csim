#include "cache.hpp"
#include "globals.hpp"
#include "bus.hpp"
#include <cassert>

namespace csim
{
    void SnoopCaches::cycle()
    {
        // validate consistency in our cache.

        // try to process request from processor.
        for (size_t proc = 0; proc < num_procs_; proc++)
        {
            Cache &cache = caches_[proc];

            if (!cache.pending_cpu_req)
                continue;

            if (!snoopbus_->isCacheTurn(proc))
            {
                continue;
            }
            // there is a pending request from processor
            if (isAHit(cache.pending_cpu_req.value(), proc))
            {
                std::cout << proc << "it is a hit " << cache.pending_cpu_req.value() << std::endl;
                // record cache hit
                stats_->cachestats[proc].hits++;

                CPUMsg cpuresp = cache.pending_cpu_req.value();
                cpuresp.msgtype = RESPONSE;
                cpus_->replyFromCache(cpuresp);
                cache.pending_cpu_req.reset();
            }
            else
            {
                // check if our turn to send on bus and send bus request

                stats_->cachestats[proc].misses++;
                BusMsg busreq;
                // Record interconnect traffic
                // TODO: handle upgrades, add moesi, mesif
                if ((coherproto_ == MESI || coherproto_ == MSI) && (cache.pending_cpu_req->inst_.command == MEM_STORE) && getCoherenceState(cache.pending_cpu_req->inst_.address, proc) == SHARED)
                {
                    busreq = BusMsg{
                        .proc_cycle_ = cycles,
                        .type_ = BUSUPGRADE,
                        .cpureq_ = cache.pending_cpu_req.value(),
                        .src_proc_ = proc,
                        .dst_proc_ = BROADCAST,
                    };
                    snoopbus_->requestFromCache(busreq);
                }
                else
                {
                    busreq = BusMsg{
                        .proc_cycle_ = cycles,
                        .type_ = (cache.pending_cpu_req->inst_.command == OperationType::MEM_LOAD) ? BusMsgType::BUSREAD : BusMsgType::BUSWRITE,
                        .cpureq_ = cache.pending_cpu_req.value(),
                        .src_proc_ = proc,
                        .dst_proc_ = BROADCAST,
                    };
                    snoopbus_->requestFromCache(busreq);
                }

                // std::cout << proc << " sent request on bus " << busreq << std::endl;
            }
        }

        snoopbus_->cycle();
    }

    void SnoopCaches::requestFromProcessor(CPUMsg cpureq)
    {
        size_t proc = cpureq.proc_;
        Cache &cache = caches_[proc];
        assert(!cache.pending_cpu_req);
        cache.pending_cpu_req = cpureq;
    }

    void SnoopCaches::requestFromBus(BusMsg busmsg)
    {
        // another processor sent a request.
        // can be rd/wr/upgr
        // respond if you have it, depends on snooping protocol.

        size_t proc = busmsg.dst_proc_;
        std::optional<BusMsg> busresp;
        switch (coherproto_)
        {
        case MI:
            busresp = requestFromBusMI(busmsg, proc);
            break;
        case MSI:
            busresp = requestFromBusMSI(busmsg, proc);
            break;
        case MESI:
            busresp = requestFromBusMESI(busmsg, proc);
            break;
        case MOESI:
            busresp = requestFromBusMOESI(busmsg, proc);
            break;
        case MESIF:
            busresp = requestFromBusMESIF(busmsg, proc);
            break;
        }
        if (busresp)
        {
            snoopbus_->replyFromCache(std::move(*busresp));
        }
    }

    void SnoopCaches::replyFromBus(BusMsg busmsg)
    {
        // another processor/Memory sent a reply.
        // can be data/shared/memory.
        // depends on coherence protocol.

        size_t proc = busmsg.dst_proc_;

        CPUMsg cpuresp;
        switch (coherproto_)
        {
        case MI:
            cpuresp = replyFromBusMI(busmsg, proc);
            break;
        case MSI:
            cpuresp = replyFromBusMSI(busmsg, proc);
            break;
        case MESI:
            cpuresp = replyFromBusMESI(busmsg, proc);
            break;
        case MOESI:
            cpuresp = replyFromBusMOESI(busmsg, proc);
            break;
        case MESIF:
            cpuresp = replyFromBusMESIF(busmsg, proc);
            break;
        }

        assert(caches_[proc].pending_cpu_req);
        assert(caches_[proc].pending_cpu_req.value() == cpuresp);
        cpus_->replyFromCache(std::move(cpuresp));
        caches_[proc].pending_cpu_req.reset();
    }

    SnoopCaches::SnoopCaches(size_t num_procs, SnoopBus *snoopbus, CPUS *cpus, CoherenceProtocol coherproto, Stats *stats) : num_procs_(num_procs), snoopbus_(snoopbus), cpus_(cpus), coherproto_(coherproto), stats_(stats)
    {
        caches_ = std::vector(num_procs_, Cache{.lines = std::unordered_map<size_t, CoherenceState>(), .pending_cpu_req = std::nullopt});
    }

    void SnoopCaches::setBus(SnoopBus *snoopbus)
    {
        snoopbus_ = snoopbus;
    }

    void SnoopCaches::setCPUs(CPUS *cpus)
    {
        cpus_ = cpus;
    }

    bool SnoopCaches::isAHit(CPUMsg &cpureq, size_t proc)
    {
        // TODO: might depend on coherence protocol
        switch (coherproto_)
        {
        case MI:
            return isAHitMI(cpureq, proc);
        case MSI:
            return isAHitMSI(cpureq, proc);
        case MESI:
            return isAHitMESI(cpureq, proc);
            break;
        case MOESI:
            return isAHitMOESI(cpureq, proc);
            break;
        case MESIF:
            return isAHitMESIF(cpureq, proc);
            break;
        }

        return true;
    }

    CoherenceState SnoopCaches::getCoherenceState(size_t address, size_t proc)
    {
        Cache &cache = caches_[proc];
        if (cache.lines.find(address) == cache.lines.end())
        {
            return INVALID;
        }
        return cache.lines[address];
    }

    void SnoopCaches::setCoherenceState(size_t address, CoherenceState newstate, size_t proc)
    {
        // record coherence invalidations
        if (getCoherenceState(address, proc) != INVALID && newstate == INVALID)
        {
            stats_->cachestats[proc].coherence_evicts++;
        }

        Cache &cache = caches_[proc];
        cache.lines[address] = newstate;
    }

    std::optional<BusMsg> SnoopCaches::requestFromBusMI(BusMsg &busreq, size_t proc)
    {
        size_t address = busreq.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busreq.type_;

        assert(busmsgtype == BUSREAD || busmsgtype == BUSWRITE);
        assert(curr_state == MODIFIED || curr_state == INVALID);

        if (busmsgtype == BUSREAD)
        {
            if (curr_state == INVALID)
            {
                return std::nullopt;
            }
            else if (curr_state == MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
        }
        else if (busmsgtype == BUSWRITE)
        {
            if (curr_state == INVALID)
            {
                return std::nullopt;
            }
            else if (curr_state == MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
        }
        return std::nullopt;
    }

    CPUMsg SnoopCaches::replyFromBusMI(BusMsg &busresp, size_t proc)
    {
        size_t address = busresp.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busresp.type_;

        assert(busmsgtype == BUSDATA || busmsgtype == MEMDATA);
        assert(curr_state == INVALID);
        setCoherenceState(address, MODIFIED, proc);

        CPUMsg cpuresp = busresp.cpureq_;
        cpuresp.msgtype = RESPONSE;

        return cpuresp;
    }

    bool SnoopCaches::isAHitMI(CPUMsg &cpureq, size_t proc)
    {
        size_t address = cpureq.inst_.address;
        CoherenceState currstate = getCoherenceState(address, proc);
        return currstate == MODIFIED;
    }
    std::optional<BusMsg> SnoopCaches::requestFromBusMSI(BusMsg &busreq, size_t proc)
    {
        size_t address = busreq.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busreq.type_;

        assert(busmsgtype == BUSREAD || busmsgtype == BUSWRITE || busmsgtype == BUSUPGRADE);
        assert(curr_state == MODIFIED || curr_state == SHARED || curr_state == INVALID);

        if (busmsgtype == BUSREAD)
        {
            if (curr_state == MODIFIED)
            {
                std::cout << proc << " M->S " << address << std::endl;
                setCoherenceState(address, SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSSHARED;
                return busresp;
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->S " << address << std::endl;
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSSHARED;
                return busresp;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BUSWRITE)
        {
            if (curr_state == MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BUSUPGRADE)
        {
            if (curr_state == MODIFIED)
            {
                std::cout << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
    CPUMsg SnoopCaches::replyFromBusMSI(BusMsg &busresp, size_t proc)
    {
        size_t address = busresp.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busresp.type_;

        assert(busmsgtype == BUSDATA || busmsgtype == MEMDATA || busmsgtype == BUSSHARED);
        assert(curr_state == SHARED || curr_state == INVALID);

        if (busmsgtype == BUSDATA || busmsgtype == MEMDATA)
        {
            setCoherenceState(address, MODIFIED, proc);
        }
        else if (busmsgtype == BUSSHARED)
        {
            assert(busresp.cpureq_.inst_.command == MEM_LOAD);
            setCoherenceState(address, SHARED, proc);
        }

        CPUMsg cpuresp = busresp.cpureq_;
        cpuresp.msgtype = RESPONSE;

        return cpuresp;
    }
    bool SnoopCaches::isAHitMSI(CPUMsg &cpureq, size_t proc)
    {
        size_t address = cpureq.inst_.address;
        CoherenceState currstate = getCoherenceState(address, proc);
        OperationType type = cpureq.inst_.command;

        assert(currstate == MODIFIED || currstate == SHARED || currstate == INVALID);
        assert(type == MEM_LOAD || type == MEM_STORE);

        if (type == MEM_LOAD)
        {
            return currstate == MODIFIED || currstate == SHARED;
        }
        else if (type == MEM_STORE)
        {
            return currstate == MODIFIED;
        }
        return false;
    }
    std::optional<BusMsg> SnoopCaches::requestFromBusMESI(BusMsg &busreq, size_t proc)
    {
        size_t address = busreq.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busreq.type_;

        assert(busmsgtype == BUSREAD || busmsgtype == BUSWRITE || busmsgtype == BUSUPGRADE);
        assert(curr_state == MODIFIED || curr_state == EXCLUSIVE || curr_state == SHARED || curr_state == INVALID);

        if (busmsgtype == BUSREAD)
        {
            if (curr_state == MODIFIED)
            {
                std::cout << proc << " M->S " << address << std::endl;
                setCoherenceState(address, SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSSHARED;
                return busresp;
            }
            else if (curr_state == EXCLUSIVE)
            {
                std::cout << proc << " E->S " << address << std::endl;
                setCoherenceState(address, SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSSHARED;
                return busresp;
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->S " << address << std::endl;
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSSHARED;
                return busresp;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BUSWRITE)
        {
            if (curr_state == MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
            else if (curr_state == EXCLUSIVE)
            {
                std::cout << proc << " E->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BUSUPGRADE)
        {
            if (curr_state == MODIFIED)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == EXCLUSIVE)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
    CPUMsg SnoopCaches::replyFromBusMESI(BusMsg &busresp, size_t proc)
    {
        size_t address = busresp.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busresp.type_;

        assert(busmsgtype == BUSDATA || busmsgtype == MEMDATA || busmsgtype == BUSSHARED);
        assert(curr_state == SHARED || curr_state == INVALID);

        if (busmsgtype == BUSDATA || busmsgtype == MEMDATA)
        {
            switch (busresp.cpureq_.inst_.command)
            {
            case MEM_LOAD:
                setCoherenceState(address, EXCLUSIVE, proc);
                break;
            case MEM_STORE:
                setCoherenceState(address, MODIFIED, proc);
                break;
            }
        }
        else if (busmsgtype == BUSSHARED)
        {
            assert(busresp.cpureq_.inst_.command == MEM_LOAD);
            setCoherenceState(address, SHARED, proc);
        }
        CPUMsg cpuresp = busresp.cpureq_;
        cpuresp.msgtype = RESPONSE;

        return cpuresp;
    }
    bool SnoopCaches::isAHitMESI(CPUMsg &cpureq, size_t proc)
    {
        size_t address = cpureq.inst_.address;
        CoherenceState currstate = getCoherenceState(address, proc);
        OperationType type = cpureq.inst_.command;

        assert(type == MEM_LOAD || type == MEM_STORE);
        assert(currstate == MODIFIED || currstate == EXCLUSIVE || currstate == SHARED || currstate == INVALID);

        switch (type)
        {
        case MEM_LOAD:
            return currstate == MODIFIED || currstate == EXCLUSIVE || currstate == SHARED;
        case MEM_STORE:
            if (currstate == EXCLUSIVE)
            {
                std::cout << proc << " E->M " << std::endl;
                setCoherenceState(address, MODIFIED, proc);
                return true;
            }
            return currstate == MODIFIED;
        }
    }
    std::optional<BusMsg> SnoopCaches::requestFromBusMOESI(BusMsg &busreq, size_t proc)
    {
        size_t address = busreq.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busreq.type_;

        assert(busmsgtype == BUSREAD || busmsgtype == BUSWRITE || busmsgtype == BUSUPGRADE);
        assert(curr_state == MODIFIED || curr_state == OWNED || curr_state == EXCLUSIVE || curr_state == SHARED || curr_state == INVALID);

        if (busmsgtype == BUSREAD)
        {
            if (curr_state == MODIFIED)
            {
                std::cout << proc << " M->O " << address << std::endl;
                setCoherenceState(address, OWNED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSSHARED;
                return busresp;
            }
            else if (curr_state == OWNED)
            {
                std::cout << proc << " O->O " << address << std::endl;
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSSHARED;
                return busresp;
            }
            else if (curr_state == EXCLUSIVE)
            {
                std::cout << proc << " E->O " << address << std::endl;
                setCoherenceState(address, OWNED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSSHARED;
                return busresp;
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->S " << address << std::endl;
                return std::nullopt;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BUSWRITE)
        {
            if (curr_state == MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
            else if (curr_state == OWNED)
            {
                std::cout << proc << " O->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
            else if (curr_state == EXCLUSIVE)
            {
                std::cout << proc << " E->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BUSUPGRADE)
        {
            if (curr_state == MODIFIED)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == OWNED)
            {
                std::cout << proc << " O->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == EXCLUSIVE)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
            return std::nullopt;
        }
        return std::nullopt;
    }
    CPUMsg SnoopCaches::replyFromBusMOESI(BusMsg &busresp, size_t proc)
    {
        size_t address = busresp.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busresp.type_;

        assert(busmsgtype == BUSDATA || busmsgtype == MEMDATA || busmsgtype == BUSSHARED);
        assert(curr_state == OWNED || curr_state == SHARED || curr_state == INVALID);

        if (busmsgtype == BUSDATA || busmsgtype == MEMDATA)
        {
            switch (busresp.cpureq_.inst_.command)
            {
            case MEM_LOAD:
                setCoherenceState(address, EXCLUSIVE, proc);
                break;
            case MEM_STORE:
                setCoherenceState(address, MODIFIED, proc);
                break;
            }
        }
        else if (busmsgtype == BUSSHARED)
        {
            assert(busresp.cpureq_.inst_.command == MEM_LOAD);
            setCoherenceState(address, SHARED, proc);
        }
        CPUMsg cpuresp = busresp.cpureq_;
        cpuresp.msgtype = RESPONSE;

        return cpuresp;
    }
    bool SnoopCaches::isAHitMOESI(CPUMsg &cpureq, size_t proc)
    {
        size_t address = cpureq.inst_.address;
        CoherenceState currstate = getCoherenceState(address, proc);
        OperationType type = cpureq.inst_.command;

        assert(type == MEM_LOAD || type == MEM_STORE);
        assert(currstate == MODIFIED || currstate == OWNED || currstate == EXCLUSIVE || currstate == SHARED || currstate == INVALID);

        switch (type)
        {
        case MEM_LOAD:
            return currstate == MODIFIED || currstate == OWNED || currstate == EXCLUSIVE || currstate == SHARED;
        case MEM_STORE:
            if (currstate == EXCLUSIVE)
            {
                std::cout << proc << " E->M " << std::endl;
                setCoherenceState(address, MODIFIED, proc);
                return true;
            }
            return currstate == MODIFIED;
        }
    }
    std::optional<BusMsg> SnoopCaches::requestFromBusMESIF(BusMsg &busreq, size_t proc)
    {
        size_t address = busreq.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busreq.type_;

        assert(busmsgtype == BUSREAD || busmsgtype == BUSWRITE || busmsgtype == BUSUPGRADE);
        assert(curr_state == MODIFIED || curr_state == EXCLUSIVE || curr_state == SHARED || curr_state == INVALID || curr_state == FORWARDER);

        if (busmsgtype == BUSREAD)
        {
            if (curr_state == MODIFIED)
            {
                std::cout << proc << " M->S " << address << std::endl;
                setCoherenceState(address, SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSSHARED;
                return busresp;
            }
            else if (curr_state == EXCLUSIVE)
            {
                std::cout << proc << " E->S " << address << std::endl;
                setCoherenceState(address, SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSSHARED;
                return busresp;
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->S " << address << std::endl;
                return std::nullopt;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
            else if (curr_state == FORWARDER)
            {
                std::cout << proc << " F->S " << address << std::endl;
                setCoherenceState(address, SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSSHARED;
                return busresp;
            }
        }
        else if (busmsgtype == BUSWRITE)
        {
            if (curr_state == MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
            else if (curr_state == EXCLUSIVE)
            {
                std::cout << proc << " E->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
            else if (curr_state == FORWARDER)
            {
                std::cout << proc << " F->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BUSDATA;
                return busresp;
            }
        }
        else if (busmsgtype == BUSUPGRADE)
        {
            if (curr_state == MODIFIED)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == EXCLUSIVE)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == INVALID)
            {
                return std::nullopt;
            }
            else if (curr_state == FORWARDER)
            {
                std::cout << proc << " F->I " << address << std::endl;
                setCoherenceState(address, INVALID, proc);
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
    CPUMsg SnoopCaches::replyFromBusMESIF(BusMsg &busresp, size_t proc)
    {
        size_t address = busresp.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busresp.type_;

        assert(busmsgtype == BUSDATA || busmsgtype == MEMDATA || busmsgtype == BUSSHARED);
        assert(curr_state == SHARED || curr_state == INVALID || curr_state == FORWARDER);

        if (busmsgtype == BUSDATA || busmsgtype == MEMDATA)
        {
            switch (busresp.cpureq_.inst_.command)
            {
            case MEM_LOAD:
                setCoherenceState(address, EXCLUSIVE, proc);
                break;
            case MEM_STORE:
                setCoherenceState(address, MODIFIED, proc);
                break;
            }
        }
        else if (busmsgtype == BUSSHARED)
        {
            assert(busresp.cpureq_.inst_.command == MEM_LOAD);
            setCoherenceState(address, FORWARDER, proc);
        }
        CPUMsg cpuresp = busresp.cpureq_;
        cpuresp.msgtype = RESPONSE;

        return cpuresp;
    }
    bool SnoopCaches::isAHitMESIF(CPUMsg &cpureq, size_t proc)
    {
        size_t address = cpureq.inst_.address;
        CoherenceState currstate = getCoherenceState(address, proc);
        OperationType type = cpureq.inst_.command;

        assert(type == MEM_LOAD || type == MEM_STORE);
        assert(currstate == MODIFIED || currstate == EXCLUSIVE || currstate == SHARED || currstate == INVALID || currstate == FORWARDER);

        switch (type)
        {
        case MEM_LOAD:
            return currstate == MODIFIED || currstate == EXCLUSIVE || currstate == SHARED || currstate == FORWARDER;
        case MEM_STORE:
            if (currstate == EXCLUSIVE)
            {
                std::cout << proc << " E->M " << address << std::endl;
                setCoherenceState(address, MODIFIED, proc);
                return true;
            }
            return currstate == MODIFIED;
        }
    }
    bool DirectoryCaches::isAHit(CPUMsg &cpureq, size_t proc)
    {
        return false;
    }
    CoherenceState DirectoryCaches::getCoherenceState(size_t address, size_t proc)
    {
        return CoherenceState();
    }
    void DirectoryCaches::setCoherenceState(size_t address, CoherenceState newstate, size_t proc)
    {
    }
    void DirectoryCaches::requestFromDirectory(DirMsg dirreq, size_t proc)
    {
    }
    void DirectoryCaches::replyFromDirectory(DirMsg dirresp, size_t proc)
    {
    }
}
