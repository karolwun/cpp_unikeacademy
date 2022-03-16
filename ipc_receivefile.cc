#include <iostream>

#include "src/ipc.hh"
#include "src/parse_args.hh"

int main(int argc, char **argv)
{
    ParseArgs parsed_args = ParseArgs(std::vector<std::string>(argv + 1, argv + argc));
    if(parsed_args.isHelp()) {
        //print_help();
    }

    IPCReceiver ipcReceiver(parsed_args.getIPCMethod(), parsed_args.getFile(), std::vector<std::string>());
    ipcReceiver.receive();
    return 0;
}