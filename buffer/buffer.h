#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <atomic>
#include <string>
#include <unistd.h>
#include <sys/uio.h>

class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;
    size_t ReadableBytes() const ;
    size_t PrependableBytes() const;

    void Retrieve(size_t len);
    void RetrieveAll() ;
    std::string RetrieveAllToStr();

    char* BeginWrite();
    char* BeginRead();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);

    ssize_t ReadFd(int fd, int &saveErrno);
    ssize_t WriteFd(int fd, int &saveErrno);

private:
    void MakeSpace(size_t len);

    std::vector<char> buffer_;
    std::atomic<size_t> readIndex_;
    std::atomic<size_t> writeIndex_;
};

#endif //BUFFER_H