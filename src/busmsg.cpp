#include "busmsg.hpp"
#include <iostream>

namespace csim
{

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
        os << " Proc Cycle: " << busmsg.proc_cycle_;
        os << " Type: " << busmsg.type_;
        os << " MemReq: " << busmsg.cpureq_;
        os << " Src: " << busmsg.src_proc_;
        os << " Dst: " << busmsg.dst_proc_;
        os << "]";
        return os;
    }
}

