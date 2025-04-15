#include "busmsg.hpp"
#include <iostream>

namespace csim
{

    std::ostream &operator<<(std::ostream &os, const BusMsgType &busmsgtype)
    {
        switch (busmsgtype)
        {
        case BUSREAD:
            os << "BUSREAD";
            break;
        case BUSWRITE:
            os << "BUSWRITE";
            break;
        case BUSDATA:
            os << "BUSDATA";
            break;
        case BUSSHARED:
            os << "BUSSHARED";
            break;
        case BUSUPGRADE:
            os << "BUSUPGRADE";
            break;
        case MEMDATA:
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
