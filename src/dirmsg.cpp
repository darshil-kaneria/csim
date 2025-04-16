#include "dirmsg.hpp"

namespace csim
{

    std::ostream &operator<<(std::ostream &os, const DirMsgType &dirmsgtype)
    {

        switch (dirmsgtype)
        {
        case DirMsgType::READ:
            os << "READ";
            break;
        case DirMsgType::WRITE:
            os << "WRITE";
            break;
        case DirMsgType::UPGRADE:
            os << "UPGRADE";
            break;
        case DirMsgType::DATA:
            os << "DATA";
            break;
        case DirMsgType::SHARED:
            os << "SHARED";
            break;
        case DirMsgType::INVALIDATE:
            os << "INVALIDATE";
            break;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const DirMsg &dirmsg)
    {
        os << "DIRMSG";
        os << " [";
        os << " Proc Cycle: " << dirmsg.proc_cycle_;
        os << " Type: " << dirmsg.type_;
        os << " MemReq: " << dirmsg.cpureq_;
        os << " Src: " << dirmsg.src_proc_;
        os << " Dst: " << dirmsg.dst_proc_;
        os << " ]";
        return os;
    }
}
