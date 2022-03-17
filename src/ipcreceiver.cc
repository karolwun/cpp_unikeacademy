#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <semaphore.h>
#include <stdexcept>
#include <sys/mman.h>
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
    else if (method == IPCMethod::shmem) {
        receiveShmem();
    }
    else if (method == IPCMethod::msgqueue) {
        receiveMsgqueue();
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

void IPCReceiver::receiveShmem()
{
    int fd = shm_open(shmem_name, O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        throw runtime_error("Shm open error " + string(strerror(errno)));
    }

    struct Shmem_control
    {
        size_t bytes_send;
        bool finished;
    };

    size_t data_size = stoi(params[0]);
    size_t buff_size = data_size + sizeof(Shmem_control);
    if (ftruncate(fd, buff_size) < 0) {
        throw runtime_error("Shm truncate error " + string(strerror(errno)));
    }
    
    char *region = static_cast<char*>(mmap(NULL, buff_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (region == MAP_FAILED) {
        throw runtime_error("mmap error " + string(strerror(errno)));
    }
    Shmem_control *shm_ctrl = reinterpret_cast<Shmem_control *>(region + data_size);

    sem_t *ssem = sem_open(sender_sem, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0);
    if (ssem == SEM_FAILED) {
        throw runtime_error("sender sem_open error " + string(strerror(errno)));
    }

    sem_t *rsem = sem_open(receiver_sem, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0);
    if (rsem == SEM_FAILED) {
        throw runtime_error("sem_open error " + string(strerror(errno)));
    } 

    sem_post(rsem);

    ofstream ofs(file, ios::out | ios::binary);
    bool loop = true;
    while(loop) {
        sem_wait(ssem);

        ofs.write(region, shm_ctrl->bytes_send);
        if (shm_ctrl->finished == 1) {
            loop = false;
        }

        sem_post(rsem);
    }
    sem_close(ssem);
    sem_close(rsem);
    sem_unlink(receiver_sem);
    shm_unlink(shmem_name);
}

void IPCReceiver::receiveMsgqueue()
{
    
}