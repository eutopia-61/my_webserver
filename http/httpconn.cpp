#include "httpconn.h"

using namespace std;

const char* HttpConn::srcDir;
std::atomic<int> HttpConn::userCount;
bool HttpConn::isET;

HttpConn::HttpConn() { 
    fd_ = -1;
    addr_ = { 0 };
    isClose_ = true;
};

HttpConn::~HttpConn() { 
    Close(); 
};

void HttpConn::init(int fd, const sockaddr_in& addr) {
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writeBuff_.RetrieveAll();
    readBuff_.RetrieveAll();
    isClose_ = false;
    printf("Client[%d](%s:%d) in, userCount:%d\n", fd_, GetIP(), GetPort(), (int)userCount);
}

void HttpConn::Close() {
    response_.UnmapFile();
    if(isClose_ == false){
        isClose_ = true; 
        userCount--;
        close(fd_);
        printf("Client[%d](%s:%d) quit, UserCount:%d\n", fd_, GetIP(), GetPort(), (int)userCount);
    }
}

int HttpConn::GetFd() const {
    return fd_;
};

struct sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}

const char* HttpConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}

int HttpConn::GetPort() const {
    return addr_.sin_port;
}

ssize_t HttpConn::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(fd_, saveErrno);
        if(len <= 0) {
            break;
        }
    } while(isET);

    // std::string str(readBuff_.curReadPtr(), readBuff_.ReadableBytes());
    // cout << str << endl;
    
    return len;
}

ssize_t HttpConn::write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iovCnt_);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len == 0) break;   /*传输结束*/
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {
            /*iov_[1]剩余内存*/
            iov_[1].iov_base = (uint8_t*)iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len) {
                writeBuff_.RetrieveAll();
                iov_[0].iov_len = 0;
            } 
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writeBuff_.Retrieve(len);
        }
    }while(isET || ToWriteBytes() > 10240);
    return len;
}

bool HttpConn::handleHTTPConn() {
    request_.Init();
    /*没有数据可读*/
    if(readBuff_.ReadableBytes() <= 0) {
        return false;
    }
    else if(request_.parse(readBuff_)) {
        printf("%s\n", request_.path().c_str());
        response_.Init(srcDir, request_.path(), request_.IsKeepAlive(), 200);
    }
    else {
        response_.Init(srcDir, request_.path(), false, 400);
    }
    
    response_.MakeResponse(writeBuff_);
    /* 响应头 */
    iov_[0].iov_base = const_cast<char*>(writeBuff_.curReadPtr());
    iov_[0].iov_len = writeBuff_.ReadableBytes();
    iovCnt_ = 1;

    /*文件*/
    if(response_.FileLen() > 0 && response_.File()) {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iovCnt_ = 2;
    }
    printf("fileszie: %ld, %d to %d\n", response_.FileLen(), iovCnt_, ToWriteBytes());
    return true;
}