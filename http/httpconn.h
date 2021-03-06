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

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn {
public:
    HttpConn();
    ~HttpConn();

    void init(int sockFd, const sockaddr_in& addr);

    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);

    void Close();

    // 获取连接信息
    int GetFd() const;
    int GetPort() const;
    const char* GetIP() const;
    sockaddr_in GetAddr() const;

    bool handleHTTPConn(); //定义处理该HTTP连接的接口，主要分为request的解析和response的生成

    /*获取已经写入的数据长度*/
    int ToWriteBytes() {
        return iov_[0].iov_len + iov_[1].iov_len;
    }

    bool IsKeepAlive() const {
        return request_.IsKeepAlive();
    }

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;
private:
    // 文件描述符和IP
    int fd_;
    struct  sockaddr_in addr_;

    bool isClose_;

    int iovCnt_;
    struct iovec iov_[2];

    Buffer readBuff_; //读缓冲区
    Buffer writeBuff_; /*写缓冲区*/

    HttpRequest request_;
    HttpResponse response_;
};


#endif //HTTP_CONN_H
