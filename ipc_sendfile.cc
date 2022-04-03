#include "src/ipcsender.hh"
#include "src/parse_args.hh"

#include <exception>
#include <iostream>

void print_help()
{
    std::cout << "help \n";
}

void sendFile(const ParseArgs & args)
{
    const size_t default_msg_size = 4096;
    size_t msg_size = default_msg_size;
    if (args.getIPCType() == IPCType::pipe) {
        IPCSender(std::make_unique<IPCPipe>(msg_size), args.getFile()).sendFile();
    }
    else if(args.getIPCType() == IPCType::shmem) {
        if (args.getParams()) {
            msg_size = std::stoul(args.getParams().value()[0]) * 1024; // Convert from kB into bytes.
        }
        IPCSender(std::make_unique<IPCShmem>(msg_size), args.getFile()).sendFile();
    }
    else if(args.getIPCType() == IPCType::socket) {
        IPCSender(std::make_unique<IPCSocket>(msg_size), args.getFile()).sendFile();
    }
    else if(args.getIPCType() == IPCType::msgqueue) {
        IPCSender(std::make_unique<IPCMsqqueue>(msg_size), args.getFile()).sendFile();
    }

}

int main(int argc, char **argv)
{
    ParseArgs parsed_args = ParseArgs(std::vector<std::string>(argv + 1, argv + argc));
    if(parsed_args.isHelp()) {
        print_help();
        return 0;
    }

    // try catch to ensure stack unwinding
    std::exception_ptr eptr;
    try {
        sendFile(parsed_args);
    }
    catch(...) {
        eptr = std::current_exception();
        std::rethrow_exception(eptr);
    }

    return 0;
}