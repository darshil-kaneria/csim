#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include "argparser.hpp"

namespace csim
{
    std::vector<std::string> split(const std::string &s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::stringstream ss(s);
        std::string token;
        while (std::getline(ss, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    void parseArgs(int argc, char *argv[], Config &config)
    {
        for (int i = 1; i < argc; ++i)
        {
            std::string arg(argv[i]);

            if (arg.substr(0, 2) != "--")
            {
                std::cerr << "Invalid argument format: " << arg << std::endl;
                continue;
            }

            size_t equal_pos = arg.find('=');
            if (equal_pos == std::string::npos)
            {
                std::cerr << "Missing '=' in argument: " << arg << std::endl;
                continue;
            }

            std::string key = arg.substr(2, equal_pos - 2);
            std::string value = arg.substr(equal_pos + 1);

            if (key == "num_procs")
            {
                config.num_procs = std::stoi(value);
            }
            else if (key == "directory")
            {
                config.directory = value;
            }
            else if (key == "cohertype")
            {
                if (value == "SNOOP" || value == "snoop")
                {
                    config.cohertype = CoherenceType::SNOOP;
                }
                else if (value == "DIRECTORY" || value == "directory")
                {
                    config.cohertype = CoherenceType::DIRECTORY;
                }
                else
                {
                    std::cerr << "Invalid Coherence Type" << std::endl;
                    exit(1);
                }
            }
            else if (key == "coherproto")
            {
                if (value == "MI" || value == "mi")
                {
                    config.coherproto = CoherenceProtocol::MI;
                }
                else if (value == "MSI" || value == "msi")
                {
                    config.coherproto = CoherenceProtocol::MSI;
                }
                else if (value == "MESI" || value == "mesi")
                {
                    config.coherproto = CoherenceProtocol::MESI;
                }
                else if (value == "MESIF" || value == "mesif")
                {
                    config.coherproto = CoherenceProtocol::MESIF;
                }
                else if (value == "MOESI" || value == "moesi")
                {
                    config.coherproto = CoherenceProtocol::MOESI;
                }
                else
                {
                    std::cerr << "Invalid Coherence Protocol" << std::endl;
                    exit(0);
                }
            }
            else if (key == "cache_line_size")
            {
                config.cache_line_size = std::stoi(value);
            }
            else if (key == "cache_size")
            {
                config.cache_size = std::stoi(value);
            }
            else if (key == "diropt")
            {
                config.diropt = (value == "true" || value == "1");
            }
            else
            {
                std::cerr << "Unknown key: " << key << std::endl;
            }
        }
    }

    void printConfig(const Config &config)
    {
        std::cout << "num_procs: " << config.num_procs << "\n";
        std::cout << "directory: " << config.directory << "\n";
        std::cout << "cohertypes: " << config.cohertype << "\n";
        std::cout << "\ncoherproto: " << config.coherproto << "\n";
        std::cout << "cache_line_size: " << config.cache_line_size << "\n";
        std::cout << "cache_size: " << config.cache_size << "\n";
        std::cout << "diropt: " << (config.diropt ? "true" : "false") << "\n";
    }
}