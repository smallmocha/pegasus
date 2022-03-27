#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <netinet/in.h>
#include <string.h>
#include "buffer/buffer.h"

class HttpConn
{
public:
    HttpConn() : connFd_(-1), isClosed(true) 
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
    }

    void Close() 
    {
        if (!isClosed) {
            close(connFd_);
            isClosed = true;
        }
    }

    int GetFd() 
    {
        return connFd_;
    }

    int Read(int &saveErrno);
    int Write(int &saveErrno);

    bool Process();

private:
    int connFd_;
    sockaddr_in addr_;
    bool isClosed;
    Buffer readBuff_;
    Buffer writeBuff_;
};

#endif