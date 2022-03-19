#pragma once

#include "ipc.hh"

#include <fstream>
#include <memory>

class IPCSender
{
public:
    IPCSender(std::unique_ptr<IPCMethod> method, std::string file);

    void sendFile();
private:
    const std::unique_ptr<IPCMethod> method;
    size_t message_size;
    std::ifstream ifile;
};