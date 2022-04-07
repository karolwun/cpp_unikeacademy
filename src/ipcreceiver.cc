#include "ipcreceiver.hh"

using namespace std;

IPCReceiver::IPCReceiver(std::unique_ptr<IPCMethod> method, const string & file): method(std::move(method))
{
    ofile.open(file, ios::binary | ios::out);
    if (!ofile) {
        throw std::runtime_error("Can't open file: " + file);        
    }
    ofile.exceptions(ifstream::badbit);
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