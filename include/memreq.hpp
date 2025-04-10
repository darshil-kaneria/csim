#pragma once

#include "trace.hpp"
#include <cstdint>

class CPU;

namespace csim
{

    struct MemReq
    {
        Instruction inst_;
        uint8_t proc_;
        uint8_t proc_seq_;
        CPU *processor_;
        MemReq(Instruction inst, uint8_t proc, uint64_t proc_seq, CPU *processor);
        bool operator==(MemReq&);
    };
}