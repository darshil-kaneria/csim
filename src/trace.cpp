#include "trace.hpp"

namespace csim
{
    bool Instruction::operator==(Instruction &other)
    {
        return this->address == other.address && this->command == other.command;
    }
}