#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>
#include "epoller/epoller.h"
#include "threadpool/threadpool.h"
#include "http/httpconn.h"
#include "timer/timer.h"

class WebServer {
public:
    WebServer(uint16_t port, int threadNum);
    ~WebServer();

    void Run();

private:
    bool InitSocket();

    void HandleNewConn();
    void HandleRead(int fd);
    void HandleWrite(int fd);
    
    void CloseConn(HttpConn &client);

    void OnRead(HttpConn &client);
    void OnWrite(HttpConn &client);

    static void SetFdNonBlock(int fd) 
    {
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    }

private:
    uint16_t port_;
    int listenFd_;
    bool isClosed_;
    std::shared_ptr<ThreadPool> threadPool_;
    Epoller epoller_;
    TimerQueue timerQueue_;
    std::unordered_map<int, HttpConn> users_;
};

#endif