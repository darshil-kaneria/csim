#include "trace.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace csim
{
    bool Instruction::operator==(Instruction &other)
    {
        return this->address == other.address && this->command == other.command;
    }

    std::optional<Instruction> TraceReader::readNextLine(size_t proc)
    {
        std::string line;
        std::ifstream &file = files[proc];
        if (std::getline(file, line))
        {
            // std::cout <<  proc << " " << line << std::endl;
            std::vector<std::string> tokens = split(line, ' ');
            // std::cout << tokens.size() << std::endl;
            // std::cout << tokens[0] << " " << tokens[1] << std::endl;
            if (tokens.size() < 2)
            {
                std::cerr << proc << " Error reading instruction" << std::endl;
                return std::nullopt;
            }
            else
            {
                Instruction ins;
                if (tokens[0] == "L")
                {
                    ins.command = OperationType::MEM_LOAD;
                }
                else if (tokens[0] == "S")
                {
                    // std::cout << "HERE" << std::endl;
                    ins.command = OperationType::MEM_STORE;
                }

                ins.address = std::stol(tokens[1]);
                return ins;
            }
        }

        return std::nullopt;
    }

    TraceReader::TraceReader(std::string directory, size_t num_procs) : num_procs_(num_procs), directory_(directory)
    {
        std::filesystem::path base = "..";
        std::filesystem::path folder = directory;

        for (size_t proc = 0; proc < num_procs; proc++)
        {
            std::filesystem::path file = std::to_string(proc) + ".txt";
            std::filesystem::path fullpath = base / folder / file;
            std::ifstream filereader(fullpath);
            if (!filereader)
            {
                std::cerr << "no file provided for processor " << proc << " filepath:" << fullpath << std::endl;
                exit(255);
            }
            files.push_back(std::move(filereader));
        }
    }

    std::vector<std::string> TraceReader::split(const std::string &line, char delim)
    {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(ss, token, delim))
        {
            tokens.push_back(token);
        }

        return tokens;
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
        os << " ]";
        return os;
    }
}