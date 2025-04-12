#include <ostream>
#include "cpumsg.hpp"

namespace csim
{

    bool CPUMsg::operator==(CPUMsg &other)
    {
        return this->inst_ == other.inst_ && this->proc_ == other.proc_ && this->proc_seq_ == other.proc_seq_;
    }

    std::ostream &operator<<(std::ostream &os, const CPUMsgType &req)
    {
        switch (req)
        {
        case REQUEST:
            os << "REQEUST";
            break;
        case RESPONSE:
            os << "RESPONSE";
            break;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const CPUMsg &cpumsg)
    {
        os << "CPUMSG: [";
        os << "Proc cycle: " << cpumsg.proc_cycle_;
        os << "Type: " << cpumsg.msgtype;
        os << " Instruction: " << cpumsg.inst_;
        os << " proc: " << cpumsg.proc_;
        os << " proc_seq: " << cpumsg.proc_seq_;
        os << "]";
        return os;
    }
}