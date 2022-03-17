#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <netinet/in.h>
#include <semaphore.h>
#include <signal.h>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "ipc.hh"

using namespace std;

IPCSender::IPCSender(IPCMethod method, string file, vector<string> params): 
                     method(method), file(std::move(file)), params(std::move(params)) {}

void IPCSender::sendIPC()
{
    if (method == IPCMethod::pipe) {
        sendPipe();
    }
    else if (method == IPCMethod::socket) {
        sendSocket();
    }
    else if (method == IPCMethod::shmem) {
        sendShmem();
    }
    else if (method == IPCMethod::msgqueue) {
        sendMsgqueue();
    }
}

void IPCSender::sendPipe()
{
    int rv = mkfifo(pipe_path, 0666);
    if (rv == 0 || errno == EEXIST) {
        ifstream ifs(file, ios::in | ios::binary);
        ofstream ofs(pipe_path, ios::out | ios::binary);
        ofs << ifs.rdbuf();
    }
    else {
        throw runtime_error("Can't create pipe. mkfifo returned with " +  string(strerror(errno)));
    }  
}

void IPCSender::sendSocket()
{
    int sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw runtime_error("Socket error " + string(strerror(errno)));
    }

    sockaddr_un serveraddr{};
    serveraddr.sun_family = AF_LOCAL;
    strncpy(serveraddr.sun_path, socket_path, strlen(socket_path));

    if (bind(sockfd, (sockaddr *) &serveraddr, sizeof(serveraddr)) != 0) {
        throw runtime_error("Bind error " + string(strerror(errno)));
    }

    if (listen(sockfd, 1) != 0) {
        throw runtime_error("Socker listen error " + string(strerror(errno)));
    }

    int clientfd = accept(sockfd, nullptr, nullptr);
    if (clientfd < 0) {
        throw runtime_error("Client accept error " + string(strerror(errno)));
    }

    ifstream ifs(file, ios::in | ios::binary | ios::ate);
    size_t fsize;
    const size_t buff_size = 4096;
    size_t sent_bytes = 0;
    char buff[buff_size];

    fsize = ifs.tellg();
    ifs.seekg (0, ios::beg);
    while (sent_bytes < fsize) {
        int bytes_read = ifs.read(buff, buff_size).gcount();
        if (send(clientfd, buff, bytes_read, 0) < 0) {
            throw runtime_error("Send to client failed " + string(strerror(errno)));
        }
        sent_bytes += buff_size;
    }

    close(sockfd);
    close(clientfd);
    unlink(socket_path);
}

void IPCSender::sendShmem()
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

    sem_post(ssem);

    ifstream ifs(file, ios::in | ios::binary);
    while(true) {
        sem_wait(rsem);

        size_t bytes_read = ifs.read(region, data_size).gcount();
        shm_ctrl->bytes_send = bytes_read;
        if (bytes_read == 0) {
            shm_ctrl->finished = true;
        }

        sem_post(ssem);

        if (bytes_read == 0) {
            break;
        }
    }
    sem_close(ssem);
    sem_close(rsem);
    sem_unlink(sender_sem);
    shm_unlink(shmem_name);
}

void IPCSender::sendMsgqueue()
{

}
// void IPCSender::sendSocket()
// {
//     ofstream ofs(sender_pid_path);
//     ofs << getpid();
//     ofs.close();

//     ifstream ifs;
//     while(1) {
//         ifs.open(receiver_pid_path);
//         if (ifs.good()) {
//             break;
//         }
//         ifs.clear();
//     }

//     //signal(SIGUSR, )

//     //int rv = kill(-1, SIGUSR1);
//     //std::cout << rv << string(strerror(errno)) << std::endl;
// }