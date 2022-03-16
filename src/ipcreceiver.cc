#include <cstring>
#include <fstream>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "ipc.hh"

using namespace std;

IPCReceiver::IPCReceiver(IPCMethod method, string file, vector<string> params): 
                     method(method), file(std::move(file)), params(std::move(params)) {}

void IPCReceiver::receive()
{
    if (method == IPCMethod::pipe) {
        receivePipe();
    }
    else if (method == IPCMethod::socket) {
        receiveSocket();
    }
}

void IPCReceiver::receivePipe()
{
    int rv = mkfifo(pipe_path, 0666);
    if (rv == 0 || errno == EEXIST) {
        ifstream ifs(pipe_path, ios::in | ios::binary);
        ofstream ofs(file, ios::out | ios::binary);
        ofs << ifs.rdbuf();
    }
    else {
        throw std::runtime_error("Can't create pipe. mkfifo returned with " +  string(strerror(errno)));
    }   
}

void IPCReceiver::receiveSocket()
{
    ofstream ofs(file, ios::out | ios::binary);

    int sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw runtime_error("Socket error " + string(strerror(errno)));
    }

    sockaddr_un serveraddr{};
    serveraddr.sun_family = AF_LOCAL;
    strncpy(serveraddr.sun_path, socket_path, strlen(socket_path));

    if (connect(sockfd, (sockaddr *)&serveraddr, sizeof(serveraddr)) != 0) {
        throw runtime_error("Connect to server error " + string(strerror(errno)));
    }

    const size_t buff_size = 4096;
    char buff[buff_size];
    int rbytes;
    while (true) {
        rbytes = recv(sockfd, buff, buff_size, 0);
        if (rbytes > 0) {
            ofs.write(buff, rbytes);
        }
        else if (rbytes == 0) {
            break;
        }
        else {
            throw runtime_error("Client recv error " + string(strerror(errno)));
        }
    }

    close(sockfd);
}