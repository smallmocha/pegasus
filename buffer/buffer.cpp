#include "buffer.h"
#include <assert.h>

Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readIndex_(0), writeIndex_(0) {}

size_t Buffer::ReadableBytes() const 
{
    return writeIndex_ - readIndex_;
}

size_t Buffer::WritableBytes() const 
{
    return buffer_.size() - writeIndex_;
}

size_t Buffer::PrependableBytes() const 
{
    return readIndex_;
}

void Buffer::Retrieve(size_t len) 
{
    assert(len <= ReadableBytes());
    readIndex_ += len;
}

void Buffer::RetrieveAll() 
{
    readIndex_ = 0;
    writeIndex_ = 0;
}

std::string Buffer::RetrieveAllToStr() 
{
    std::string str(buffer_.data(), ReadableBytes());
    RetrieveAll();
    return str;
}

char* Buffer::BeginWrite() 
{
    return buffer_.data() + writeIndex_;
}

char* Buffer::BeginRead() 
{
    return buffer_.data() + readIndex_;
}

void Buffer::Append(const std::string& str) 
{
    Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len) 
{
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const char* str, size_t len)
{
    assert(str);
    if(WritableBytes() < len) {
        MakeSpace(len);
    }
    assert(WritableBytes() >= len);
    std::copy(str, str + len, BeginWrite());
    writeIndex_ += len;
}

ssize_t Buffer::ReadFd(int fd, int &saveErrno)
{
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = WritableBytes();
    /* 分散读， 保证数据全部读完 */
    iov[0].iov_base = buffer_.data() + writeIndex_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    ssize_t len = readv(fd, iov, 2);
    if(len < 0) {
        saveErrno = errno;
        return len;
    } else if(static_cast<size_t>(len) <= writable) {
        writeIndex_ += len;
    } else {
        writeIndex_ = buffer_.size();
        Append(buff, len - writable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int &saveErrno)
{
    size_t readSize = ReadableBytes();
    ssize_t len = write(fd, BeginRead(), readSize);
    if(len < 0) {
        saveErrno = errno;
        return len;
    } 
    readIndex_ += len;
    return len;
}

void Buffer::MakeSpace(size_t len)
{
    if(WritableBytes() + PrependableBytes() < len) {
        buffer_.resize(writeIndex_ + len + 1);
    } 
    else {
        size_t readable = ReadableBytes();
        std::copy(BeginRead(), BeginWrite(), buffer_.data());
        readIndex_ = 0;
        writeIndex_ = readIndex_ + readable;
    }
}
