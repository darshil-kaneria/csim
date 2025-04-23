#include "cache.hpp"
#include "globals.hpp"
#include "bus.hpp"
#include <cassert>
#include "directory.hpp"

namespace csim
{
    void SnoopCaches::cycle()
    {
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
                // std::cout << proc << "it is a hit " << cache.pending_cpu_req.value() << std::endl;
                // record cache hit
                stats_->cachestats[proc].hits++;

                CPUMsg cpuresp = cache.pending_cpu_req.value();
                cpuresp.msgtype = CPUMsgType::RESPONSE;
                cpus_->replyFromCache(cpuresp);
                cache.pending_cpu_req.reset();
            }
            else
            {
                stats_->cachestats[proc].misses++;
                BusMsg busreq;

                if ((coherproto_ == CoherenceProtocol::MESIF || coherproto_ == CoherenceProtocol::MOESI || coherproto_ == CoherenceProtocol::MESI || coherproto_ == CoherenceProtocol::MSI) && (cache.pending_cpu_req->inst_.command == OperationType::MEM_STORE) && getCoherenceState(cache.pending_cpu_req->inst_.address, proc) == CoherenceState::SHARED)
                {
                    busreq = BusMsg{
                        .proc_cycle_ = cycles,
                        .type_ = BusMsgType::UPGRADE,
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
                        .type_ = (cache.pending_cpu_req->inst_.command == OperationType::MEM_LOAD) ? BusMsgType::READ : BusMsgType::WRITE,
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
        case CoherenceProtocol::MI:
            busresp = requestFromBusMI(busmsg, proc);
            break;
        case CoherenceProtocol::MSI:
            busresp = requestFromBusMSI(busmsg, proc);
            break;
        case CoherenceProtocol::MESI:
            busresp = requestFromBusMESI(busmsg, proc);
            break;
        case CoherenceProtocol::MOESI:
            busresp = requestFromBusMOESI(busmsg, proc);
            break;
        case CoherenceProtocol::MESIF:
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
        case CoherenceProtocol::MI:
            cpuresp = replyFromBusMI(busmsg, proc);
            break;
        case CoherenceProtocol::MSI:
            cpuresp = replyFromBusMSI(busmsg, proc);
            break;
        case CoherenceProtocol::MESI:
            cpuresp = replyFromBusMESI(busmsg, proc);
            break;
        case CoherenceProtocol::MOESI:
            cpuresp = replyFromBusMOESI(busmsg, proc);
            break;
        case CoherenceProtocol::MESIF:
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
        case CoherenceProtocol::MI:
            return isAHitMI(cpureq, proc);
        case CoherenceProtocol::MSI:
            return isAHitMSI(cpureq, proc);
        case CoherenceProtocol::MESI:
            return isAHitMESI(cpureq, proc);
        case CoherenceProtocol::MOESI:
            return isAHitMOESI(cpureq, proc);
        case CoherenceProtocol::MESIF:
            return isAHitMESIF(cpureq, proc);
        }

        return true;
    }

    CoherenceState SnoopCaches::getCoherenceState(size_t address, size_t proc)
    {
        Cache &cache = caches_[proc];
        if (cache.lines.find(address) == cache.lines.end())
        {
            return CoherenceState::INVALID;
        }
        return cache.lines[address];
    }

    void SnoopCaches::setCoherenceState(size_t address, CoherenceState newstate, size_t proc)
    {
        // record coherence invalidations
        if (getCoherenceState(address, proc) != CoherenceState::INVALID && newstate == CoherenceState::INVALID)
        {
            stats_->cachestats[proc].evictions++;
        }

        Cache &cache = caches_[proc];
        cache.lines[address] = newstate;
    }

    std::optional<BusMsg> SnoopCaches::requestFromBusMI(BusMsg &busreq, size_t proc)
    {
        size_t address = busreq.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busreq.type_;

        assert(busmsgtype == BusMsgType::READ || busmsgtype == BusMsgType::WRITE);
        assert(curr_state == CoherenceState::MODIFIED || curr_state == CoherenceState::INVALID);

        if (busmsgtype == BusMsgType::READ)
        {
            if (curr_state == CoherenceState::INVALID)
            {
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
        }
        else if (busmsgtype == BusMsgType::WRITE)
        {
            if (curr_state == CoherenceState::INVALID)
            {
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
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

        assert(busmsgtype == BusMsgType::DATA || busmsgtype == BusMsgType::MEMDATA);
        assert(curr_state == CoherenceState::INVALID);
        setCoherenceState(address, CoherenceState::MODIFIED, proc);

        CPUMsg cpuresp = busresp.cpureq_;
        cpuresp.msgtype = CPUMsgType::RESPONSE;

        return cpuresp;
    }

    bool SnoopCaches::isAHitMI(CPUMsg &cpureq, size_t proc)
    {
        size_t address = cpureq.inst_.address;
        CoherenceState currstate = getCoherenceState(address, proc);
        return currstate == CoherenceState::MODIFIED;
    }
    std::optional<BusMsg> SnoopCaches::requestFromBusMSI(BusMsg &busreq, size_t proc)
    {
        size_t address = busreq.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busreq.type_;

        assert(busmsgtype == BusMsgType::READ || busmsgtype == BusMsgType::WRITE || busmsgtype == BusMsgType::UPGRADE);
        assert(curr_state == CoherenceState::MODIFIED || curr_state == CoherenceState::SHARED || curr_state == CoherenceState::INVALID);

        if (busmsgtype == BusMsgType::READ)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cout << proc << " M->S " << address << std::endl;
                setCoherenceState(address, CoherenceState::SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::SHARED;
                return busresp;
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->S " << address << std::endl;
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::SHARED;
                return busresp;
            }
            else if (curr_state == CoherenceState::INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BusMsgType::WRITE)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
            else if (curr_state == CoherenceState::INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BusMsgType::UPGRADE)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cout << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::INVALID)
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

        assert(busmsgtype == BusMsgType::DATA || busmsgtype == BusMsgType::MEMDATA || busmsgtype == BusMsgType::SHARED);
        assert(curr_state == CoherenceState::SHARED || curr_state == CoherenceState::INVALID);

        if (busmsgtype == BusMsgType::DATA || busmsgtype == BusMsgType::MEMDATA)
        {
            setCoherenceState(address, CoherenceState::MODIFIED, proc);
        }
        else if (busmsgtype == BusMsgType::SHARED)
        {
            assert(busresp.cpureq_.inst_.command == OperationType::MEM_LOAD);
            setCoherenceState(address, CoherenceState::SHARED, proc);
        }

        CPUMsg cpuresp = busresp.cpureq_;
        cpuresp.msgtype = CPUMsgType::RESPONSE;

        return cpuresp;
    }
    bool SnoopCaches::isAHitMSI(CPUMsg &cpureq, size_t proc)
    {
        size_t address = cpureq.inst_.address;
        CoherenceState currstate = getCoherenceState(address, proc);
        OperationType type = cpureq.inst_.command;

        assert(currstate == CoherenceState::MODIFIED || currstate == CoherenceState::SHARED || currstate == CoherenceState::INVALID);
        assert(type == OperationType::MEM_LOAD || type == OperationType::MEM_STORE);

        if (type == OperationType::MEM_LOAD)
        {
            return currstate == CoherenceState::MODIFIED || currstate == CoherenceState::SHARED;
        }
        else if (type == OperationType::MEM_STORE)
        {
            return currstate == CoherenceState::MODIFIED;
        }
        return false;
    }
    std::optional<BusMsg> SnoopCaches::requestFromBusMESI(BusMsg &busreq, size_t proc)
    {
        size_t address = busreq.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busreq.type_;

        assert(busmsgtype == BusMsgType::READ || busmsgtype == BusMsgType::WRITE || busmsgtype == BusMsgType::UPGRADE);
        assert(curr_state == CoherenceState::MODIFIED || curr_state == CoherenceState::EXCLUSIVE || curr_state == CoherenceState::SHARED || curr_state == CoherenceState::INVALID);

        if (busmsgtype == BusMsgType::READ)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cout << proc << " M->S " << address << std::endl;
                setCoherenceState(address, CoherenceState::SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::SHARED;
                return busresp;
            }
            else if (curr_state == CoherenceState::EXCLUSIVE)
            {
                std::cout << proc << " E->S " << address << std::endl;
                setCoherenceState(address, CoherenceState::SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::SHARED;
                return busresp;
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->S " << address << std::endl;
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::SHARED;
                return busresp;
            }
            else if (curr_state == CoherenceState::INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BusMsgType::WRITE)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
            else if (curr_state == CoherenceState::EXCLUSIVE)
            {
                std::cout << proc << " E->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
            else if (curr_state == CoherenceState::INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BusMsgType::UPGRADE)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == CoherenceState::EXCLUSIVE)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::INVALID)
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

        assert(busmsgtype == BusMsgType::DATA || busmsgtype == BusMsgType::MEMDATA || busmsgtype == BusMsgType::SHARED);
        assert(curr_state == CoherenceState::SHARED || curr_state == CoherenceState::INVALID);

        if (busmsgtype == BusMsgType::DATA || busmsgtype == BusMsgType::MEMDATA)
        {
            switch (busresp.cpureq_.inst_.command)
            {
            case OperationType::MEM_LOAD:
                setCoherenceState(address, CoherenceState::EXCLUSIVE, proc);
                break;
            case OperationType::MEM_STORE:
                setCoherenceState(address, CoherenceState::MODIFIED, proc);
                break;
            }
        }
        else if (busmsgtype == BusMsgType::SHARED)
        {
            assert(busresp.cpureq_.inst_.command == OperationType::MEM_LOAD);
            setCoherenceState(address, CoherenceState::SHARED, proc);
        }
        CPUMsg cpuresp = busresp.cpureq_;
        cpuresp.msgtype = CPUMsgType::RESPONSE;

        return cpuresp;
    }
    bool SnoopCaches::isAHitMESI(CPUMsg &cpureq, size_t proc)
    {
        size_t address = cpureq.inst_.address;
        CoherenceState currstate = getCoherenceState(address, proc);
        OperationType type = cpureq.inst_.command;

        assert(type == OperationType::MEM_LOAD || type == OperationType::MEM_STORE);
        assert(currstate == CoherenceState::MODIFIED || currstate == CoherenceState::EXCLUSIVE || currstate == CoherenceState::SHARED || currstate == CoherenceState::INVALID);

        switch (type)
        {
        case OperationType::MEM_LOAD:
            return currstate == CoherenceState::MODIFIED || currstate == CoherenceState::EXCLUSIVE || currstate == CoherenceState::SHARED;
        case OperationType::MEM_STORE:
            if (currstate == CoherenceState::EXCLUSIVE)
            {
                std::cout << proc << " E->M " << std::endl;
                setCoherenceState(address, CoherenceState::MODIFIED, proc);
                return true;
            }
            return currstate == CoherenceState::MODIFIED;
        }
    }
    std::optional<BusMsg> SnoopCaches::requestFromBusMOESI(BusMsg &busreq, size_t proc)
    {
        size_t address = busreq.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busreq.type_;

        assert(busmsgtype == BusMsgType::READ || busmsgtype == BusMsgType::WRITE || busmsgtype == BusMsgType::UPGRADE);
        assert(curr_state == CoherenceState::MODIFIED || curr_state == CoherenceState::OWNED || curr_state == CoherenceState::EXCLUSIVE || curr_state == CoherenceState::SHARED || curr_state == CoherenceState::INVALID);

        if (busmsgtype == BusMsgType::READ)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cout << proc << " M->O " << address << std::endl;
                setCoherenceState(address, CoherenceState::OWNED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::SHARED;
                return busresp;
            }
            else if (curr_state == CoherenceState::OWNED)
            {
                std::cout << proc << " O->O " << address << std::endl;
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::SHARED;
                return busresp;
            }
            else if (curr_state == CoherenceState::EXCLUSIVE)
            {
                std::cout << proc << " E->O " << address << std::endl;
                setCoherenceState(address, CoherenceState::OWNED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::SHARED;
                return busresp;
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->S " << address << std::endl;
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BusMsgType::WRITE)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
            else if (curr_state == CoherenceState::OWNED)
            {
                std::cout << proc << " O->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
            else if (curr_state == CoherenceState::EXCLUSIVE)
            {
                std::cout << proc << " E->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
            else if (curr_state == CoherenceState::INVALID)
            {
                return std::nullopt;
            }
        }
        else if (busmsgtype == BusMsgType::UPGRADE)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == CoherenceState::OWNED)
            {
                std::cout << proc << " O->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::EXCLUSIVE)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::INVALID)
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

        assert(busmsgtype == BusMsgType::DATA || busmsgtype == BusMsgType::MEMDATA || busmsgtype == BusMsgType::SHARED);
        assert(curr_state == CoherenceState::OWNED || curr_state == CoherenceState::SHARED || curr_state == CoherenceState::INVALID);

        if (busmsgtype == BusMsgType::DATA || busmsgtype == BusMsgType::MEMDATA)
        {
            switch (busresp.cpureq_.inst_.command)
            {
            case OperationType::MEM_LOAD:
                setCoherenceState(address, CoherenceState::EXCLUSIVE, proc);
                break;
            case OperationType::MEM_STORE:
                setCoherenceState(address, CoherenceState::MODIFIED, proc);
                break;
            }
        }
        else if (busmsgtype == BusMsgType::SHARED)
        {
            assert(busresp.cpureq_.inst_.command == OperationType::MEM_LOAD);
            setCoherenceState(address, CoherenceState::SHARED, proc);
        }
        CPUMsg cpuresp = busresp.cpureq_;
        cpuresp.msgtype = CPUMsgType::RESPONSE;

        return cpuresp;
    }
    bool SnoopCaches::isAHitMOESI(CPUMsg &cpureq, size_t proc)
    {
        size_t address = cpureq.inst_.address;
        CoherenceState currstate = getCoherenceState(address, proc);
        OperationType type = cpureq.inst_.command;

        assert(type == OperationType::MEM_LOAD || type == OperationType::MEM_STORE);
        assert(currstate == CoherenceState::MODIFIED || currstate == CoherenceState::OWNED || currstate == CoherenceState::EXCLUSIVE || currstate == CoherenceState::SHARED || currstate == CoherenceState::INVALID);

        switch (type)
        {
        case OperationType::MEM_LOAD:
            return currstate == CoherenceState::MODIFIED || currstate == CoherenceState::OWNED || currstate == CoherenceState::EXCLUSIVE || currstate == CoherenceState::SHARED;
        case OperationType::MEM_STORE:
            if (currstate == CoherenceState::EXCLUSIVE)
            {
                std::cout << proc << " E->M " << std::endl;
                setCoherenceState(address, CoherenceState::MODIFIED, proc);
                return true;
            }
            return currstate == CoherenceState::MODIFIED;
        }
    }
    std::optional<BusMsg> SnoopCaches::requestFromBusMESIF(BusMsg &busreq, size_t proc)
    {
        size_t address = busreq.cpureq_.inst_.address;
        CoherenceState curr_state = getCoherenceState(address, proc);
        BusMsgType busmsgtype = busreq.type_;

        assert(busmsgtype == BusMsgType::READ || busmsgtype == BusMsgType::WRITE || busmsgtype == BusMsgType::UPGRADE);
        assert(curr_state == CoherenceState::MODIFIED || curr_state == CoherenceState::EXCLUSIVE || curr_state == CoherenceState::SHARED || curr_state == CoherenceState::INVALID || curr_state == CoherenceState::FORWARDER);

        if (busmsgtype == BusMsgType::READ)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cout << proc << " M->S " << address << std::endl;
                setCoherenceState(address, CoherenceState::SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::SHARED;
                return busresp;
            }
            else if (curr_state == CoherenceState::EXCLUSIVE)
            {
                std::cout << proc << " E->S " << address << std::endl;
                setCoherenceState(address, CoherenceState::SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::SHARED;
                return busresp;
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->S " << address << std::endl;
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::INVALID)
            {
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::FORWARDER)
            {
                std::cout << proc << " F->S " << address << std::endl;
                setCoherenceState(address, CoherenceState::SHARED, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::SHARED;
                return busresp;
            }
        }
        else if (busmsgtype == BusMsgType::WRITE)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cout << proc << " M->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
            else if (curr_state == CoherenceState::EXCLUSIVE)
            {
                std::cout << proc << " E->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::INVALID)
            {
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::FORWARDER)
            {
                std::cout << proc << " F->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                BusMsg busresp = busreq;
                busresp.dst_proc_ = busresp.src_proc_;
                busresp.src_proc_ = proc;
                busresp.type_ = BusMsgType::DATA;
                return busresp;
            }
        }
        else if (busmsgtype == BusMsgType::UPGRADE)
        {
            if (curr_state == CoherenceState::MODIFIED)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == CoherenceState::EXCLUSIVE)
            {
                std::cerr << "something is broken" << std::endl;
                assert(false);
            }
            else if (curr_state == CoherenceState::SHARED)
            {
                std::cout << proc << " S->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::INVALID)
            {
                return std::nullopt;
            }
            else if (curr_state == CoherenceState::FORWARDER)
            {
                std::cout << proc << " F->I " << address << std::endl;
                setCoherenceState(address, CoherenceState::INVALID, proc);
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

        assert(busmsgtype == BusMsgType::DATA || busmsgtype == BusMsgType::MEMDATA || busmsgtype == BusMsgType::SHARED);
        assert(curr_state == CoherenceState::SHARED || curr_state == CoherenceState::INVALID || curr_state == CoherenceState::FORWARDER);

        if (busmsgtype == BusMsgType::DATA || busmsgtype == BusMsgType::MEMDATA)
        {
            switch (busresp.cpureq_.inst_.command)
            {
            case OperationType::MEM_LOAD:
                setCoherenceState(address, CoherenceState::EXCLUSIVE, proc);
                break;
            case OperationType::MEM_STORE:
                setCoherenceState(address, CoherenceState::MODIFIED, proc);
                break;
            }
        }
        else if (busmsgtype == BusMsgType::SHARED)
        {
            assert(busresp.cpureq_.inst_.command == OperationType::MEM_LOAD);
            setCoherenceState(address, CoherenceState::FORWARDER, proc);
        }
        CPUMsg cpuresp = busresp.cpureq_;
        cpuresp.msgtype = CPUMsgType::RESPONSE;

        return cpuresp;
    }
    bool SnoopCaches::isAHitMESIF(CPUMsg &cpureq, size_t proc)
    {
        size_t address = cpureq.inst_.address;
        CoherenceState currstate = getCoherenceState(address, proc);
        OperationType type = cpureq.inst_.command;

        assert(type == OperationType::MEM_LOAD || type == OperationType::MEM_STORE);
        assert(currstate == CoherenceState::MODIFIED || currstate == CoherenceState::EXCLUSIVE || currstate == CoherenceState::SHARED || currstate == CoherenceState::INVALID || currstate == CoherenceState::FORWARDER);

        switch (type)
        {
        case OperationType::MEM_LOAD:
            return currstate == CoherenceState::MODIFIED || currstate == CoherenceState::EXCLUSIVE || currstate == CoherenceState::SHARED || currstate == CoherenceState::FORWARDER;
        case OperationType::MEM_STORE:
            if (currstate == CoherenceState::EXCLUSIVE)
            {
                std::cout << proc << " E->M " << address << std::endl;
                setCoherenceState(address, CoherenceState::MODIFIED, proc);
                return true;
            }
            return currstate == CoherenceState::MODIFIED;
        }
    }

    bool DirectoryCaches::isAHit(CPUMsg &cpureq, size_t proc)
    {
        size_t address = cpureq.inst_.address;
        CoherenceState currstate = getCoherenceState(address, proc);
        OperationType type = cpureq.inst_.command;

        assert(type == OperationType::MEM_LOAD || type == OperationType::MEM_STORE);
        assert(currstate == CoherenceState::MODIFIED || currstate == CoherenceState::EXCLUSIVE || currstate == CoherenceState::SHARED || currstate == CoherenceState::INVALID);

        switch (type)
        {
        case OperationType::MEM_LOAD:
            return currstate == CoherenceState::MODIFIED || currstate == CoherenceState::EXCLUSIVE || currstate == CoherenceState::SHARED;
        case OperationType::MEM_STORE:
            if (currstate == CoherenceState::EXCLUSIVE)
            {
                std::cout << proc << " E->M " << std::endl;
                setCoherenceState(address, CoherenceState::MODIFIED, proc);
                return true;
            }
            return currstate == CoherenceState::MODIFIED;
        }
    }

    CoherenceState DirectoryCaches::getCoherenceState(size_t address, size_t proc)
    {
        Cache &cache = caches_[proc];
        if (cache.lines.find(address) == cache.lines.end())
        {
            return CoherenceState::INVALID;
        }
        return cache.lines[address];
    }

    void DirectoryCaches::setCoherenceState(size_t address, CoherenceState newstate, size_t proc)
    {
        // record coherence invalidations
        if (getCoherenceState(address, proc) != CoherenceState::INVALID && newstate == CoherenceState::INVALID)
        {
            stats_->cachestats[proc].evictions++;
        }

        Cache &cache = caches_[proc];
        cache.lines[address] = newstate;
    }

    DirectoryCaches::DirectoryCaches(size_t num_procs, Directory *directory, CPUS *cpus, Stats *stats) : num_procs_(num_procs), directory_(directory), cpus_(cpus), stats_(stats)
    {
        caches_ = std::vector(num_procs_, Cache{.lines = std::unordered_map<size_t, CoherenceState>(), .pending_cpu_req = std::nullopt});
    }

    void DirectoryCaches::cycle()
    {
        // validate consistency in our cache.

        // try to process request from processor.
        for (size_t proc = 0; proc < num_procs_; proc++)
        {
            Cache &cache = caches_[proc];

            if (!cache.pending_cpu_req)
                continue;

            if (isAHit(cache.pending_cpu_req.value(), proc))
            {
                // record cache hit
                stats_->cachestats[proc].hits++;

                CPUMsg cpuresp = cache.pending_cpu_req.value();
                cpuresp.msgtype = CPUMsgType::RESPONSE;
                cpus_->replyFromCache(cpuresp);
                cache.pending_cpu_req.reset();
            }
            else
            {
                stats_->cachestats[proc].misses++;
                DirMsg dirreq;

                if (cache.pending_cpu_req->inst_.command == OperationType::MEM_STORE && getCoherenceState(cache.pending_cpu_req->inst_.address, proc) == CoherenceState::SHARED)
                {
                    dirreq = DirMsg{
                        .proc_cycle_ = cycles,
                        .type_ = DirMsgType::UPGRADE,
                        .cpureq_ = cache.pending_cpu_req.value(),
                        .src_proc_ = proc,
                        .dst_proc_ = DIRECTORY};
                    directory_->requestFromCache(dirreq);
                }
                else
                {
                    dirreq = DirMsg{
                        .proc_cycle_ = cycles,
                        .type_ = (cache.pending_cpu_req->inst_.command == OperationType::MEM_LOAD) ? DirMsgType::READ : DirMsgType::WRITE,
                        .cpureq_ = cache.pending_cpu_req.value(),
                        .src_proc_ = proc,
                        .dst_proc_ = DIRECTORY};
                    directory_->requestFromCache(dirreq);
                }
            }
        }
    }
    void DirectoryCaches::requestFromProcessor(CPUMsg cpureq)
    {
        size_t proc = cpureq.proc_;
        Cache &cache = caches_[proc];
        assert(!cache.pending_cpu_req);
        cache.pending_cpu_req = cpureq;
    }

    void DirectoryCaches::requestFromDirectory(DirMsg dirreq)
    {
        size_t proc = dirreq.dst_proc_;
        DirMsgType type = dirreq.type_;
        size_t address = dirreq.cpureq_.inst_.address;

        // request from directory only comes to downgrade to shared or invalidate.
        assert(type == DirMsgType::SHARED || type == DirMsgType::INVALIDATE);

        CoherenceState currstate = getCoherenceState(address, proc);
        if (type == DirMsgType::SHARED)
        {
            assert(currstate == CoherenceState::MODIFIED || currstate == CoherenceState::EXCLUSIVE);
            std::cout << proc << currstate << "-> S" << std::endl;
            setCoherenceState(address, CoherenceState::SHARED, proc);
        }
        else if (type == DirMsgType::INVALIDATE)
        {
            assert(currstate == CoherenceState::MODIFIED || currstate == CoherenceState::EXCLUSIVE || currstate == CoherenceState::SHARED);
            std::cout << proc << currstate << "-> I" << std::endl;
            setCoherenceState(address, CoherenceState::INVALID, proc);
        }
    }
    void DirectoryCaches::replyFromDirectory(DirMsg dirresp)
    {
        size_t proc = dirresp.dst_proc_;
        DirMsgType type = dirresp.type_;
        size_t address = dirresp.cpureq_.inst_.address;
        OperationType optype = dirresp.cpureq_.inst_.command;

        CoherenceState curr_state = getCoherenceState(address, proc);

        assert(type == DirMsgType::DATA || type == DirMsgType::SHARED);
        assert(curr_state == CoherenceState::SHARED || curr_state == CoherenceState::INVALID);

        if (type == DirMsgType::DATA)
        {
            switch (optype)
            {
            case OperationType::MEM_LOAD:
                setCoherenceState(address, CoherenceState::EXCLUSIVE, proc);
                break;
            case OperationType::MEM_STORE:
                setCoherenceState(address, CoherenceState::MODIFIED, proc);
                break;
            }
        }
        else if (type == DirMsgType::SHARED)
        {
            assert(optype == OperationType::MEM_LOAD);
            setCoherenceState(address, CoherenceState::SHARED, proc);
        }

        CPUMsg cpuresp = dirresp.cpureq_;
        cpuresp.msgtype = CPUMsgType::RESPONSE;

        assert(caches_[proc].pending_cpu_req);
        assert(caches_[proc].pending_cpu_req.value() == cpuresp);

        cpus_->replyFromCache(std::move(cpuresp));
        caches_[proc].pending_cpu_req.reset();
    }

    void DirectoryCaches::setDirectory(Directory *directory)
    {
        directory_ = directory;
    }
    void DirectoryCaches::setCPUs(CPUS *cpus)
    {
        cpus_ = cpus;
    }
    std::ostream &operator<<(std::ostream &os, const CoherenceState &state)
    {
        switch (state)
        {
        case CoherenceState::MODIFIED:
            os << "M";
            break;
        case CoherenceState::INVALID:
            os << "I";
            break;
        case CoherenceState::SHARED:
            os << "S";
            break;
        case CoherenceState::EXCLUSIVE:
            os << "E";
            break;
        case CoherenceState::OWNED:
            os << "O";
            break;
        case CoherenceState::FORWARDER:
            os << "F";
            break;
        }
        return os;
    }
    std::ostream &operator<<(std::ostream &os, const CoherenceProtocol &protocol)
    {
        switch (protocol)
        {
        case CoherenceProtocol::MI:
            os << "MI";
            break;
        case CoherenceProtocol::MSI:
            os << "MSI";
            break;
        case CoherenceProtocol::MESI:
            os << "MESI";
            break;
        case CoherenceProtocol::MOESI:
            os << "MOESI";
            break;
        case CoherenceProtocol::MESIF:
            os << "MESIF";
            break;
        }
        return os;
    }
}
