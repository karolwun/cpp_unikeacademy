#include "../src/ipcreceiver.hh"
#include "../src/ipcsender.hh"

#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

class TestIPC : public ::testing::Test
{
protected:
    inline static const size_t msg_size = 1024;
    inline static const std::string small_file_s = "small_file_s";
    inline static const std::string big_file_s = "big_file_s";
    inline static const std::string small_file_d = "small_file_d";
    inline static const std::string big_file_d = "big_file_d";

    TestIPC()
    {
        createRandFile(100, small_file_s);
        createRandFile(1000000, big_file_s);
    }

    ~TestIPC()
    {
        std::remove(small_file_s.c_str());
        std::remove(big_file_s.c_str());
        std::remove(small_file_d.c_str());
        std::remove(big_file_d.c_str());
    }

    bool filesEqual(const std::string & f_path1, const std::string & f_path2)
    {
        std::stringstream cmd;
        cmd << "cmp " << f_path1 << " " << f_path2;
        int rv = std::system(cmd.str().c_str());
        return WEXITSTATUS(rv) == 0;
    }

private:
    void createRandFile(size_t size, const std::string & file_path)
    {
        std::stringstream command; 
        command << "dd if=/dev/urandom of=" << file_path << " bs=" << size << " count=1 2> /dev/null";
        std::system(command.str().c_str());
    }
};

template<typename T>
void send_thread(size_t msg_size, const std::string & filepath)
{
    IPCSender(std::make_unique<T>(msg_size), filepath).sendFile();
}

template<typename T>
void receive_thread(size_t msg_size, const std::string & filepath)
{
    IPCReceiver(std::make_unique<T>(msg_size), filepath).receiveFile();
}


TEST_F(TestIPC, SendPipeSmallFile)
{
    std::thread s_thread(send_thread<IPCPipe>, msg_size, small_file_s);
    std::thread r_thread(receive_thread<IPCPipe>, msg_size, small_file_d);
    s_thread.join();
    r_thread.join();
    EXPECT_TRUE(filesEqual(small_file_s, small_file_d));
}

TEST_F(TestIPC, SendPipeBigFile)
{
    std::thread s_thread(send_thread<IPCPipe>, msg_size, big_file_s);
    std::thread r_thread(receive_thread<IPCPipe>, msg_size, big_file_d);
    s_thread.join();
    r_thread.join();
    EXPECT_TRUE(filesEqual(big_file_s, big_file_d));
}

TEST_F(TestIPC, SendShmSmallFile)
{
    std::thread s_thread(send_thread<IPCShmem>, msg_size, small_file_s);
    std::thread r_thread(receive_thread<IPCShmem>, msg_size, small_file_d);
    s_thread.join();
    r_thread.join();
    EXPECT_TRUE(filesEqual(small_file_s, small_file_d));
}

TEST_F(TestIPC, SendShmBigFile)
{
    std::thread s_thread(send_thread<IPCShmem>, msg_size, big_file_s);
    std::thread r_thread(receive_thread<IPCShmem>, msg_size, big_file_d);
    s_thread.join();
    r_thread.join();
    EXPECT_TRUE(filesEqual(big_file_s, big_file_d));
}

TEST_F(TestIPC, SendSocketSmallFile)
{
    std::thread s_thread(send_thread<IPCSocket>, msg_size, small_file_s);
    std::thread r_thread(receive_thread<IPCSocket>, msg_size, small_file_d);
    s_thread.join();
    r_thread.join();
    EXPECT_TRUE(filesEqual(small_file_s, small_file_d));
}

TEST_F(TestIPC, SendSocketBigFile)
{
    std::thread s_thread(send_thread<IPCSocket>, msg_size, small_file_s);
    std::thread r_thread(receive_thread<IPCSocket>, msg_size, small_file_d);
    s_thread.join();
    r_thread.join();
    EXPECT_TRUE(filesEqual(small_file_s, small_file_d));
}

TEST_F(TestIPC, SendMsqqueueSmallFile)
{
    std::thread s_thread(send_thread<IPCMsqqueue>, msg_size, small_file_s);
    std::thread r_thread(receive_thread<IPCMsqqueue>, msg_size, small_file_d);
    s_thread.join();
    r_thread.join();
    EXPECT_TRUE(filesEqual(small_file_s, small_file_d));   
}

TEST_F(TestIPC, SendMsqqueueBigFile)
{
    std::thread s_thread(send_thread<IPCMsqqueue>, msg_size, big_file_s);
    std::thread r_thread(receive_thread<IPCMsqqueue>, msg_size, big_file_d);
    s_thread.join();
    r_thread.join();
    EXPECT_TRUE(filesEqual(big_file_s, big_file_d));   
}