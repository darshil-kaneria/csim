#include "busmsg.hpp"
#include <iostream>

namespace csim {

    std::ostream &operator<<(std::ostream &os, const BusMsgState &busmsgstate)
    {
        switch (busmsgstate)
        {
        case BusMsgState::NONE:
            os << "NONE";
            break;
        case BusMsgState::QUEUED:
            os << "QUEUED";
            break;
        case BusMsgState::CACHE_DELAY:
            os << "CACHE_DELAY";
            break;
        case BusMsgState::REQUESTED_DATA:
            os << "REQUESTED_DATA";
            break;
        case BusMsgState::CACHE_PROVIDED_DATA:
            os << "CACHE_PROVIDED_DATA";
            break;
        case BusMsgState::MEMORY_PROVIDED_DATA:
            os << "MEMORY_PROVIDED_DATA";
            break;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const BusMsgType &busmsgtype)
    {
        switch (busmsgtype)
        {
        case BusMsgType::BUSREAD:
            os << "BUSREAD";
            break;
        case BusMsgType::BUSWRITE:
            os << "BUSWRITE";
            break;
        case BusMsgType::BUSDATA:
            os << "BUSDATA";
            break;
        case BusMsgType::BUSSHARED:
            os << "BUSSHARED";
            break;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const BusMsg &busmsg)
    {
        os << "BUSMSG";
        os << "[";
        os << " Type: " << busmsg.type;
        os << " State: " << busmsg.state;
        os << " MemReq: " << busmsg.memreq;
        os << " Src: " << int(busmsg.src_proc_);
        os << " Dst: " << int(busmsg.dst_proc_);
        os << "]";
        return os;
    }
}