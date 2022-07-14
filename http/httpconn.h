#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/types.h>
#include <sys/uio.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <atomic>

class HttpConn {
public:
    HttpConn();
    ~HttpConn();

    void init(int sockFd, const sockaddr_in& addr);

    size_t read(int* saveErrno);
    size_t write(int* saveErrno);

    void Close();

    // 获取连接信息
    int GetFd() const;
    int GetPort() const;
    const char* GetIP() const;
    sockaddr_in GetAddr() const;

    static std::atomic<int> userCount;
    
private:
    // 文件描述符和IP
    int fd_;
    struct  sockaddr_in addr_;

    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];

};


#endif //HTTP_CONN_H
