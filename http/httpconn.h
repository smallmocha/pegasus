#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <string>
#include <string.h>
#include <netinet/in.h>
#include "buffer/buffer.h"
#include "timer/timer.h"
#include "http/http_requst.h"
#include "http/http_response.h"
#include "log/log.h"
#include <arpa/inet.h>

class HttpConn
{
public:
    HttpConn() : connFd_(-1), isClosed(true), timer_(nullptr), iovCnt_(0)
    {
        bzero(&addr_, sizeof(addr_));
    }

    ~HttpConn()
    {
        Close();
    }

    void Init(int fd, const sockaddr_in &addr)
    {
        connFd_ = fd;
        addr_ = addr;
        isClosed = false;
        readBuff_.RetrieveAll();
        writeBuff_.RetrieveAll();
        LOG_INFO("Client[%d](%s:%d) in", connFd_, inet_ntoa(addr_.sin_addr), addr_.sin_port);
    }

    void Close()
    {
        httpResponse_.UnmapFile();
        if (!isClosed) {
            close(connFd_);
            isClosed = true;
        }
    }

    int GetFd()
    {
        return connFd_;
    }

    void SetTimer(Timer *timer)
    {
        timer_ = timer;
    }

    Timer *GetTimer()
    {
        return timer_;
    }

    int Read(int &saveErrno);
    int Write(int &saveErrno);

    bool Process();

    static std::string srcDir_;

private:
    int connFd_;
    sockaddr_in addr_;
    bool isClosed;
    Timer *timer_;
    Buffer readBuff_;
    Buffer writeBuff_;
    
    int iovCnt_;
    iovec iov_[2];

    HttpRequst httpRequst_;
    HttpResponse httpResponse_;
};

#endif