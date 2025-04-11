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
        uint8_t proc_seq_;
        CPU *processor_;
        MemReq(Instruction inst, uint8_t proc, uint64_t proc_seq, CPU *processor);
        bool operator==(MemReq &);
    };
}