#pragma once

#include "ipc.hh"

#include <fstream>
#include <memory>

class IPCReceiver
{
public:
    IPCReceiver(std::unique_ptr<IPCMethod> method, std::string file);

    void receiveFile();

private:
    const std::unique_ptr<IPCMethod> method;
    size_t message_size;
    std::ofstream ofile;
};