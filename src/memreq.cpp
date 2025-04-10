#include "memreq.hpp"

namespace csim
{

    MemReq::MemReq(Instruction inst, uint8_t proc, uint64_t proc_seq, CPU *processor) : inst_(inst), proc_(proc), proc_seq_(proc_seq), processor_(processor) {}

    bool MemReq::operator==(MemReq &other)
    {
        return this->inst_ == other.inst_ && this->proc_ == other.proc_ && this->proc_seq_ == other.proc_seq_ && this->processor_ == other.processor_;
    }
}