#pragma once

#include <string>
#include <vector>

enum class IPCMethod {NONE, pipe, shmem, socket, msgqueue};

static const char *pipe_path = "/tmp/fifo";
static const char *socket_path = "/tmp/socket";
static const char *shmem_name = "shmem_ipc";
static const char *sender_sem = "/sendersem";
static const char *receiver_sem = "/receiversem";
static const char *mq_name = "/mqname";

class IPCSender
{
public:
    IPCSender(IPCMethod method, std::string file, std::vector<std::string> params);

    void sendIPC();
private:
    const IPCMethod method;
    const std::string file;
    const std::vector<std::string> params;

    void sendPipe();
    void sendSocket();
    void sendShmem();
    void sendMsgqueue();
};

class IPCReceiver
{
public:
    IPCReceiver(IPCMethod method, std::string file, std::vector<std::string> params);

    void receive();
private:
    const IPCMethod method;
    const std::string file;
    const std::vector<std::string> params;

    void receivePipe();
    void receiveSocket();
    void receiveShmem();
    void receiveMsgqueue();
};