#pragma once

#include <string>
#include <iostream>
namespace csim
{
    // TODO: Ensure to get the block address.

    const size_t MASK = 0xFFFFFFFFFFFFFFC0;

    enum class OperationType
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
        size_t cache_line_size_;
        std::vector<std::ifstream> files;
        TraceReader(std::string directory_, size_t num_procs_, size_t cache_line_size_);
        std::vector<std::string> split(const std::string &line, char delim);
    };

    std::ostream &operator<<(std::ostream &os, const OperationType &op);

    std::ostream &operator<<(std::ostream &os, const Instruction &ins);
}