#pragma once

#include "trace.hpp"
#include <cstdint>

namespace csim
{
    struct CPU;

    enum CPUMsgType
    {
        REQUEST,
        RESPONSE,
    };

    struct CPUMsg
    {
        size_t proc_cycle_;
        CPUMsgType msgtype;
        Instruction inst_;
        size_t proc_;
        size_t proc_seq_;
        bool operator==(CPUMsg &other);
    };

    std::ostream &operator<<(std::ostream &os, const CPUMsgType &req);
    std::ostream &operator<<(std::ostream &os, const CPUMsg &req);

}