#include "ipcsender.hh"

using namespace std;

IPCSender::IPCSender(unique_ptr<IPCMethod> method, string file) : method(std::move(method)) 
{
    message_size = this->method->getMaxMsgSize();
    ifile.open(file, ios::binary | ios::in);
    ifile.exceptions(ifstream::badbit);
}

void IPCSender::sendFile()
{    
    while(true) {
        vector<char> packet(message_size);
        size_t bytes_read = ifile.read(packet.data(), message_size).gcount();

        packet.resize(bytes_read);
        method->sendPacket(packet);

        if (bytes_read == 0) {
            break;
        }
    }
}

