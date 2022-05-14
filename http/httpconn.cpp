#include "httpconn.h"
#include "log/log.h"

std::string HttpConn::srcDir_;

int HttpConn::Read(int &saveErrno)
{
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(connFd_, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (true);
    return len;
}

int HttpConn::Write(int &saveErrno)
{
    ssize_t len = -1;
    do {
        len = writev(connFd_, iov_, iovCnt_);
        if (len <= 0) {
            saveErrno = errno;
            break;
        }
        if (ToWriteBytes() == 0) {
            break;
        } else if (static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = static_cast<char *>(iov_[1].iov_base) + len - iov_[0].iov_len;
            iov_[1].iov_len -= len - iov_[0].iov_len;
            if (iov_[0].iov_len > 0) {
                iov_[0].iov_len = 0;
                writeBuff_.RetrieveAll();
            }
        } else {
            iov_[0].iov_base = static_cast<char *>(iov_[0].iov_base) + len;
            iov_[0].iov_len -= len;
            writeBuff_.Retrieve(len);
        }
    } while (true);
    return len;
}

bool HttpConn::Process()
{
    httpRequst_.Init();
    if (readBuff_.ReadableBytes() <= 0) {
        return false;
    }
    if (httpRequst_.Parse(readBuff_)) {
        httpResponse_.Init(srcDir_, httpRequst_.Path(), 200);
    } else {
        httpResponse_.Init(srcDir_, httpRequst_.Path(), 400);
    }

    httpResponse_.MakeResponse(writeBuff_);

    iov_[0].iov_base = const_cast<char *>(writeBuff_.BeginRead());
    iov_[0].iov_len = writeBuff_.ReadableBytes();
    iovCnt_ = 1;

    if (httpResponse_.File() != nullptr && httpResponse_.FileLen() > 0) {
        iov_[1].iov_base = httpResponse_.File();
        iov_[1].iov_len = httpResponse_.FileLen();
        iovCnt_ = 2;
    }
    LOG_INFO("filesize(%d), iovCnt(%d), toWriteBytes(%d)", httpResponse_.FileLen() , iovCnt_, ToWriteBytes());
    return true;
}
