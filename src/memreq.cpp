#include <ostream>
#include "memreq.hpp"

namespace csim
{

    bool MemReq::operator==(MemReq &other)
    {
        return this->inst_ == other.inst_ && this->proc_ == other.proc_ && this->proc_seq_ == other.proc_seq_ && this->processor_ == other.processor_;
    }

    std::ostream &operator<<(std::ostream &os, const MemReq &memreq)
    {
        os << "MEMREQ: [";
        os << " Instruction: " << memreq.inst_;
        os << " proc: " << (int)memreq.proc_;
        os << " proc_seq: " << (int)memreq.proc_seq_;
        os << "]";
        return os;
    }
}