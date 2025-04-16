#include "busmsg.hpp"
#include <iostream>

namespace csim
{

    std::ostream &operator<<(std::ostream &os, const BusMsgType &busmsgtype)
    {
        switch (busmsgtype)
        {
        case BusMsgType::READ:
            os << "READ";
            break;
        case BusMsgType::WRITE:
            os << "WRITE";
            break;
        case BusMsgType::DATA:
            os << "DATA";
            break;
        case BusMsgType::SHARED:
            os << "SHARED";
            break;
        case BusMsgType::UPGRADE:
            os << "UPGRADE";
            break;
        case BusMsgType::MEMDATA:
            os << "MEMDATA";
            break;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const BusMsg &busmsg)
    {
        os << "BUSMSG";
        os << " [";
        os << " Proc Cycle: " << busmsg.proc_cycle_;
        os << " Type: " << busmsg.type_;
        os << " MemReq: " << busmsg.cpureq_;
        os << " Src: " << busmsg.src_proc_;
        os << " Dst: " << busmsg.dst_proc_;
        os << " ]";
        return os;
    }
}
