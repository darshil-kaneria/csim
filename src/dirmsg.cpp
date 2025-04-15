#include "dirmsg.hpp"

namespace csim
{

    std::ostream &operator<<(std::ostream &os, const DirMsgType &dirmsgtype)
    {

        switch (dirmsgtype)
        {
        case DIRREAD:
            os << "DIRREAD";
            break;
        case DIRWRITE:
            os << "DIRWRITE";
            break;
        case DIRUPGRADE:
            os << "DIRUPGRADE";
            break;
        case DIRDATA:
            os << "DIRDATA";
            break;
        case DIRSHARED:
            os << "DIRSHARED";
            break;
        case DIRINVALIDATE:
            os << "DIRINVALIDATE";
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
