#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <netinet/in.h>
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

void IPCSender::sendshmem()
{
    int fd = shm_open(shmem_name, O_RDWR, 0644);
    if (fd < 0) {
        throw runtime_error("Shm open error " + string(strerror(errno)));
    }   

	fprintf(stderr,"Shared Mem Descriptor: fd=%d\n", fd);

	assert (fd>0);

	struct stat sb;

	fstat(fd, &sb);
	off_t length = sb.st_size ;

	u_char *ptr = (u_char *) mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	fprintf(stderr, "Shared Mem Address: %p [0..%lu]\n", ptr, length-1);
	assert (ptr);

	// hexdump first 100 bytes
	fprintf(stdout,"First 100 bytes:\n");
	for(i=0; i<100; i++)
		fprintf(stdout, "%02X%s", ptr[i], (i%25==24)?("\n"):(" ") );

	// change 1st byte
	ptr[ 0 ] = 'H' ;
}

void IPCSender::sendmsgqueue()
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