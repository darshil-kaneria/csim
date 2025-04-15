
#include <cstdint>
#include "cpumsg.hpp"

namespace csim
{
    enum DirMsgType
    {
        DIRREAD,
        DIRWRITE,
        DIRUPGRADE,
        DIRDATA,
        DIRSHARED,
        DIRINVALIDATE,
    };

    struct DirMsg
    {
        size_t proc_cycle_;
        DirMsgType type_;
        CPUMsg cpureq_; // The cpu request that triggered this bus packet originally (if any)
        size_t src_proc_;
        size_t dst_proc_;
    };

    std::ostream &operator<<(std::ostream &os, const DirMsgType &busmsgtype);
    std::ostream &operator<<(std::ostream &os, const DirMsg &busmsg);
}