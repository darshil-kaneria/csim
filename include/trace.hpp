#pragma once

#include <string>
#include <iostream>
namespace csim
{
    // TODO: Ensure to get the block address.

    enum OperationType
    {
        MEM_LOAD,
        MEM_STORE
    };

    struct Instruction
    {
        OperationType command;
        size_t address;
        bool operator==(Instruction &);
    };

    struct TraceReader
    {
        std::optional<Instruction> readNextLine(size_t proc);
        size_t num_procs_;
        std::string directory_;
    };

    std::ostream &operator<<(std::ostream &os, const OperationType &op);

    std::ostream &operator<<(std::ostream &os, const Instruction &ins);
}