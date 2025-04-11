#include "trace.hpp"

namespace csim
{
    bool Instruction::operator==(Instruction &other)
    {
        return this->address == other.address && this->command == other.command;
    }

    Instruction TraceReader::readNextLine(int proc_num)
    {
        return Instruction{};
    }

    std::ostream &operator<<(std::ostream &os, const OperationType &op)
    {
        switch (op)
        {
        case OperationType::MEM_LOAD:
            os << "LOAD";
            break;
        case OperationType::MEM_STORE:
            os << "STORE";
            break;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const Instruction &ins)
    {
        os << "INS: [";
        os << " Command: " << ins.command;
        os << " Address: " << (int)ins.address;
        os << "]";
        return os;
    }
}