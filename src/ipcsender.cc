#include "ipcsender.hh"

using namespace std;

IPCSender::IPCSender(unique_ptr<IPCMethod> method, string file) : method(std::move(method)) 
{
    message_size = method->getMaxMsgSize();
    ifile.open(file, ios::binary | ios::in);
    ifile.exceptions(ifstream::failbit | ifstream::badbit);
}

void IPCSender::sendFile()
{
    vector<char> packet(message_size);
    while(true) {
        size_t bytes_read = ifile.read(packet.data(), message_size).gcount();

        if (bytes_read == 0) {
            break;
        }

        packet.resize(bytes_read);
        method->sendPacket(packet);
    }
}

