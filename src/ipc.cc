#include "ipc.hh"

#include <cstring>
#include <fcntl.h>
#include <mqueue.h>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace std;

const std::string IPCPipe::pipe_path = "/tmp/fifo";

IPCPipe::IPCPipe(size_t msg_size) : msg_size(msg_size)
{
    int rv = mkfifo(pipe_path.c_str(), 0666);
    if (rv != 0 && errno != EEXIST) {
        throw runtime_error("create pipe error " +  string(strerror(errno)));
    }
}

IPCPipe::~IPCPipe()
{
    close(fd);
    unlink(pipe_path.c_str());
}

void IPCPipe::sendPacket(const vector<char> & packet) 
{
    if (!pipe_opened_to_write) {
        openPipeWr();
    }

    if (write(fd, packet.data(), packet.size()) < 0) {
        throw runtime_error("write pipe error " +  string(strerror(errno)));        
    }
}

vector<char> IPCPipe::receivePacket()
{
    if (!pipe_opened_to_read) {
        openPipeRd();
    }

    vector<char> packet(msg_size);

    ssize_t bytes_received = read(fd, packet.data(), msg_size);
    if (bytes_received < 0) {
        throw runtime_error("Pipe read error " + string(strerror(errno)));
    }

    packet.resize(bytes_received);
    return packet;
}

size_t IPCPipe::getMaxMsgSize() const
{
    return msg_size;
}

void IPCPipe::openPipeWr()
{
    fd = open(pipe_path.c_str(), O_WRONLY);
    if (fd < 0) {
        throw runtime_error("open pipe error " +  string(strerror(errno)));
    }
    pipe_opened_to_write = true;
}

void IPCPipe::openPipeRd()
{
    fd = open(pipe_path.c_str(), O_RDONLY);
    if (fd < 0) {
        throw runtime_error("open pipe error " +  string(strerror(errno)));
    }
    pipe_opened_to_read = true;
}    

///////////////////////////////////////////////////////////////////////////////

const std::string IPCMsqqueue::mq_name = "/mqname";

IPCMsqqueue::IPCMsqqueue(size_t msg_size) : msg_size(msg_size)
{
    mq_attr attrs;
    attrs.mq_maxmsg = 1;
    attrs.mq_msgsize = msg_size;

    mq_desc = mq_open(mq_name.c_str(), O_CREAT | O_RDWR, 0666, &attrs);
    if (mq_desc == -1) {
        throw runtime_error("mq_open error " + string(strerror(errno)));
    }
}

void IPCMsqqueue::sendPacket(const vector<char> & packet)
{
        if (mq_send(mq_desc, packet.data(), packet.size(), 0) == -1) {
            throw runtime_error("mq_send error " + string(strerror(errno)));
        }
}

vector<char> IPCMsqqueue::receivePacket()
{
    vector<char> packet(msg_size);

    ssize_t bytes_received = mq_receive(mq_desc, packet.data(), msg_size, 0);
    if (bytes_received < 0) {
        throw runtime_error("mq_receive error " + string(strerror(errno)));
    }

    packet.resize(bytes_received);
    return packet;
}

size_t IPCMsqqueue::getMaxMsgSize() const
{
    return msg_size;
}

IPCMsqqueue::~IPCMsqqueue()
{
    mq_close(mq_desc);
    mq_unlink(mq_name.c_str());
}

///////////////////////////////////////////////////////////////////////////////

const std::string IPCSocket::socket_path = "/tmp/socket";
const std::string IPCSocket::server_sem_path = "/serversem";

IPCSocket::IPCSocket(size_t packet_size) : packet_size(packet_size)
{
    sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw runtime_error("Socket error " + string(strerror(errno)));
    }

    serveraddr.sun_family = AF_LOCAL;
    strncpy(serveraddr.sun_path, socket_path.c_str(), socket_path.size());

    server_sem = sem_open(server_sem_path.c_str(), O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0);
    if (server_sem == SEM_FAILED) {
        throw runtime_error("server sem_open error " + string(strerror(errno)));
    }
}

void IPCSocket::sendPacket(const vector<char> & packet)
{
    if (!server_running) {
        runServer();
    }

    if (send(clientfd, packet.data(), packet.size(), 0) < 0) {
            throw runtime_error("Send to client failed " + string(strerror(errno)));
    }
}

vector<char> IPCSocket::receivePacket()
{
    if (!client_connected) {
        clientConnect();
    }

    vector<char> packet(packet_size);

    ssize_t bytes_received = recv(sockfd, packet.data(), packet_size, 0);
    if (bytes_received < 0) {
        throw runtime_error("Client recv error " + string(strerror(errno)));
    }

    packet.resize(bytes_received);
    return packet;
}

void IPCSocket::runServer()
{
    if (bind(sockfd, (sockaddr *) &serveraddr, sizeof(serveraddr)) != 0) {
        throw runtime_error("Bind error " + string(strerror(errno)));
    }

    if (listen(sockfd, 1) != 0) {
        throw runtime_error("Socker listen error " + string(strerror(errno)));
    }

    sem_post(server_sem);

    clientfd = accept(sockfd, nullptr, nullptr);
    if (clientfd < 0) {
        throw runtime_error("Client accept error " + string(strerror(errno)));
    }

    server_running = true;
}

void IPCSocket::clientConnect()
{
    sem_wait(server_sem);
    if (connect(sockfd, (sockaddr *)&serveraddr, sizeof(serveraddr)) != 0) {
        throw runtime_error("Connect to server error " + string(strerror(errno)));
    }

    client_connected = true;   
}

size_t IPCSocket::getMaxMsgSize() const
{
    return packet_size;
}

IPCSocket::~IPCSocket()
{
    close(clientfd);
    close(sockfd);
    sem_unlink(server_sem_path.c_str());
    unlink(socket_path.c_str());
}

///////////////////////////////////////////////////////////////////////////////

const std::string IPCShmem::shmem_name = "shmem_ipc";
const std::string IPCShmem::sender_sem_path = "/sendersem";
const std::string IPCShmem::receiver_sem_path = "/receiversem";

IPCShmem::IPCShmem(size_t buff_size) : buff_size(buff_size)
{

    int fd = shm_open(shmem_name.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        throw runtime_error("Shm open error " + string(strerror(errno)));
    }

    size_t shmem_size = buff_size + sizeof(Shmem_control);
    if (ftruncate(fd, shmem_size) < 0) {
        throw runtime_error("Shm truncate error " + string(strerror(errno)));
    }
    
    region = static_cast<char*>(mmap(NULL, shmem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (region == MAP_FAILED) {
        throw runtime_error("mmap error " + string(strerror(errno)));
    }
    shm_ctrl = reinterpret_cast<Shmem_control *>(region + buff_size);

    sender_sem = sem_open(sender_sem_path.c_str(), O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0);
    if (sender_sem == SEM_FAILED) {
        throw runtime_error("sender sem_open error " + string(strerror(errno)));
    }

    receiver_sem = sem_open(receiver_sem_path.c_str(), O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 1);
    if (receiver_sem == SEM_FAILED) {
        throw runtime_error("sem_open error " + string(strerror(errno)));
    }    
}

void IPCShmem::sendPacket(const vector<char> & packet) 
{
    sem_wait(receiver_sem);

    memcpy(region, packet.data(), packet.size());
    shm_ctrl->bytes_send = packet.size();
 
    sem_post(sender_sem);
}

vector<char> IPCShmem::receivePacket()
{
    vector<char> packet(buff_size);

    sem_wait(sender_sem);

    memcpy(packet.data(), region, shm_ctrl->bytes_send);
    packet.resize(shm_ctrl->bytes_send);

    sem_post(receiver_sem);

    return packet;
}

size_t IPCShmem::getMaxMsgSize() const
{
    return buff_size;
}

IPCShmem::~IPCShmem()
{
    sem_close(sender_sem);
    sem_close(receiver_sem);
    sem_unlink(sender_sem_path.c_str());
    sem_unlink(receiver_sem_path.c_str());
    shm_unlink(shmem_name.c_str());
}