#pragma once

#include <cstdint>
#include "cpumsg.hpp"

namespace csim
{

    const size_t DIRECTORY = 3000;

    enum class DirMsgType
    {
        READ,
        WRITE,
        UPGRADE,
        DATA,
        SHARED,
        INVALIDATE,
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