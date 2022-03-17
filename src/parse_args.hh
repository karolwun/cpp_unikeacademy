#pragma once

#include <string>
#include <vector>

#include "ipc.hh"

class ParseArgs
{
public:
    explicit ParseArgs(const std::vector<std::string> & args)
    {
        for(unsigned i = 0; i < args.size(); i++) {
            if (args[i] == "--help") {
                help = true;
                break;
            }
            else if (args[i] == "--file") {
                file = args[i+1];
                i++;
            }
            else if (args[i] == "--pipe") {
                ipc_method = IPCMethod::pipe;
            }
            else if (args[i] == "--shm") {
                ipc_method = IPCMethod::shmem;
                method_params.push_back(args[i+1]);
                i++;
            }
            else if (args[i] == "--socket") {
                ipc_method = IPCMethod::socket;

            }
            else if (args[i] == "--msqqueue") {
                ipc_method = IPCMethod::msgqueue;
            }
            else { // wrong argument
                help = true;
                return;
            }
        }

        if(ipc_method == IPCMethod::NONE) {
            help = true;
        }
    }

    bool isHelp()
    {
        return help;
    }

    IPCMethod getIPCMethod()
    {
        return ipc_method;
    }

    std::string getFile()
    {
        return file;
    }

    std::vector<std::string> getParams()
    {
        return method_params;
    }

private:
    bool help;
    IPCMethod ipc_method{};
    std::string file;
    std::vector<std::string> method_params;
};