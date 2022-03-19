#include "ipcreceiver.hh"

using namespace std;

IPCReceiver::IPCReceiver(std::unique_ptr<IPCMethod> method, string file): method(std::move(method))
{
    ofile.open(file, ios::binary | ios::out);
    ofile.exceptions(ifstream::failbit | ifstream::badbit);
}

void IPCReceiver::receiveFile()
{
    
    while(true) {
        vector<char> packet {method->receivePacket()};

        if (packet.size() == 0) {
            break;
        }

        ofile.write(packet.data(), packet.size());
    }
}