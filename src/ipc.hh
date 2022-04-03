#pragma once

#include <mqueue.h>
#include <semaphore.h>
#include <string>
#include <sys/un.h>
#include <vector>

class IPCMethod
{
public:
    virtual void sendPacket(const std::vector<char> & packet) = 0;
    virtual std::vector<char> receivePacket() = 0;
    virtual size_t getMaxMsgSize() const = 0;
    virtual ~IPCMethod() {}
};

class IPCPipe : public IPCMethod
{
public:
    IPCPipe(size_t msg_size);
    virtual void sendPacket(const std::vector<char> & packet);
    virtual std::vector<char> receivePacket();
    virtual size_t getMaxMsgSize() const;
    virtual ~IPCPipe();

private:
    static const std::string pipe_path;
    size_t msg_size;
    int fd = -1;
    bool pipe_opened_to_write = false;
    bool pipe_opened_to_read = false;

    void openPipeWr();
    void openPipeRd();
};

class IPCMsqqueue : public IPCMethod
{
public:
    IPCMsqqueue(size_t msg_size);
    virtual void sendPacket(const std::vector<char> & packet);
    virtual std::vector<char> receivePacket();
    virtual size_t getMaxMsgSize() const;
    virtual ~IPCMsqqueue();

private:
    static const std::string mq_name;
    size_t msg_size;
    mqd_t mq_desc = -1;
};

class IPCSocket : public IPCMethod
{
public:
    IPCSocket(size_t packet_size);
    virtual void sendPacket(const std::vector<char> & packet);
    virtual std::vector<char> receivePacket();
    virtual size_t getMaxMsgSize() const;
    virtual ~IPCSocket();

private:
    static const std::string socket_path;
    static const std::string server_sem_path;
    size_t packet_size;
    int sockfd = -1;
    bool server_running = false;
    bool client_connected = false;

    sockaddr_un serveraddr = {};
    int clientfd = -1;
    sem_t *server_sem = nullptr;

    void runServer();
    void clientConnect();
};

class IPCShmem : public IPCMethod
{
public:
    IPCShmem(size_t buff_size);
    virtual void sendPacket(const std::vector<char> & packet);
    virtual std::vector<char> receivePacket();
    virtual size_t getMaxMsgSize() const;
    virtual ~IPCShmem();

private:
    static const std::string shmem_name;
    static const std::string sender_sem_path;
    static const std::string receiver_sem_path;
    size_t buff_size;

    struct Shmem_control
    {
        size_t bytes_send;
    };

    char *region = nullptr;
    sem_t *sender_sem = nullptr;
    sem_t *receiver_sem = nullptr;
    Shmem_control *shm_ctrl = nullptr;
};
