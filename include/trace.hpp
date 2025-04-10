# pragma once

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
        int address;
        bool operator==(Instruction&);
    };

    class TraceReader
    {
    public:
        Instruction readNextLine(int proc_num);
        

    private:
        int proc_num;
    };

   
}