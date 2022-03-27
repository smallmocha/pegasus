#include "httpconn.h"
#include <string>

int HttpConn::Read(int &saveErrno)
{
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(connFd_, saveErrno);
        if (len <= 0) {
            break;
        }
        printf("recv msg:\n%s\n", std::string(readBuff_.BeginRead(), readBuff_.ReadableBytes()).c_str());
    } while (true);
    return len;
}

int HttpConn::Write(int &saveErrno)
{
    ssize_t len = -1;
    do {
        len = writeBuff_.WriteFd(connFd_, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (true);
    return len;
}

bool HttpConn::Process()
{
    if (readBuff_.ReadableBytes() <= 0) {
        return false;
    }
    readBuff_.RetrieveAll();
    std::string str = "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: 7\r\n\r\npegasus";
    writeBuff_.Append(str);
    return true;
}