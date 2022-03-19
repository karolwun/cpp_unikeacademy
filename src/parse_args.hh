#pragma once

#include <string>
#include <vector>

enum class IPCType {NONE, pipe, shmem, socket, msgqueue};

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
                ipc_type = IPCType::pipe;
            }
            else if (args[i] == "--shm") {
                ipc_type = IPCType::shmem;
                method_params.push_back(args[i+1]);
                i++;
            }
            else if (args[i] == "--socket") {
                ipc_type = IPCType::socket;

            }
            else if (args[i] == "--msqqueue") {
                ipc_type = IPCType::msgqueue;
            }
            else { // wrong argument
                help = true;
                return;
            }
        }

        if(ipc_type == IPCType::NONE) {
            help = true;
        }
    }

    bool isHelp()
    {
        return help;
    }

    IPCType getIPCType() const
    {
        return ipc_type;
    }

    std::string getFile() const
    {
        return file;
    }

    std::vector<std::string> getParams() const
    {
        return method_params;
    }

private:
    bool help = false;
    IPCType ipc_type = IPCType::NONE;
    std::string file;
    std::vector<std::string> method_params;
};