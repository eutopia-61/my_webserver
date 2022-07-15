#ifndef BUFFER_H_
#define BUFFER_H_

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>
#include <assert.h>

class Buffer
{
public:
    Buffer(int initBufferSize = 1024);
    ~Buffer() = default;

    size_t WriteableBytes() const;  //缓存区中还可以写入的字节数
    size_t ReadableBytes() const;   //缓存区中还可以读取的字节数
    size_t PrependableBytes() const;  //缓存区已经读取的字节数

    void EnsureWriteable(size_t len);   /*保证将数据写入缓存区*/

    const char* Peek() const;   // 获取读位置指针
    
    /*获取当前写指针*/
    const char* BeginWriteConst() const;
    char* BeginWrite();

    void HasWritten(size_t len);       /*更新写位置*/
    void Retrieve(size_t len);  /*更新读位置*/
    void RetrieveUntil(const char* end);   /*将读指针直接更新到指定位置*/
    void RetrieveAll();  /*清除缓冲区*/   
    std::string RetrieveAllToStr(); /*将缓冲区数据转换成string，并清除缓冲区*/

    // 将数据写入缓冲区
    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    //IO操作的读与写接口
    ssize_t ReadFd(int fd, int *Errno);
    ssize_t WriteFd(int fd, int *Errno);
private:
    char* BeginPtr_();  //返回指向缓冲区初始位置的指针
    const char* BeginPtr_() const;

    void MakeSpace_(size_t len);    //缓冲区见不够时扩容

    std::vector<char> buffer_; //缓存区

    // 用于指示读写指针
    std::atomic<std::size_t> readPos_;
    std::atomic<std::size_t> writePos_;
};

#endif