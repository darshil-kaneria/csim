#pragma once

#include "trace.hpp"
#include <cstdint>

namespace csim
{
    class CPU;

    struct MemReq
    {
        Instruction inst_;
        uint8_t proc_;
        uint64_t proc_seq_;
        CPU *processor_;
        bool operator==(MemReq &other);
    };
    std::ostream& operator<<(std::ostream& os, const MemReq& req);


}