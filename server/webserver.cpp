#include "webserver.h"
#include <signal.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/unistd.h>

WebServer::WebServer(uint16_t port, int threadNum) : port_(port), listenFd_(-1), isClosed_(false), threadPool_(std::make_shared<ThreadPool>())
{
    assert(threadPool_);
    threadPool_->Init(threadNum);
    if (!InitSocket()) {
        isClosed_ = true;
    }
}

WebServer::~WebServer()
{
    isClosed_ = true;
    close(listenFd_);
}

void WebServer::HandleNewConn()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int connFd = accept(listenFd_, reinterpret_cast<sockaddr *>(&addr), &len);
    if (connFd < 0) {
        return;
    }
    printf("new connetion ip:%s fd:%d\n", inet_ntoa(addr.sin_addr), connFd);
    users_[connFd].Init(connFd, addr);
    epoller_.AddFd(connFd, EPOLLIN | EPOLLET);
    SetFdNonBlock(connFd);
}

void WebServer::HandleRead(int fd)
{
    printf("epollin event, fd:%d\n", fd);
    assert(users_.count(fd) > 0);
    threadPool_->AddTask(std::bind(&WebServer::OnRead, this, std::ref(users_[fd])));
}

void WebServer::HandleWrite(int fd)
{
    printf("epollout event, fd=%d\n", fd);
    assert(users_.count(fd) > 0);
    threadPool_->AddTask(std::bind(&WebServer::OnWrite, this, std::ref(users_[fd])));
}

void WebServer::CloseConn(HttpConn &client)
{
    epoller_.DelFd(client.GetFd());
    client.Close();
}

void WebServer::OnRead(HttpConn &client)
{
    int err = 0;
    int ret = client.Read(err);
    if (ret <= 0 && err != EAGAIN) {
        CloseConn(client);
    } else {
        if (client.Process()) {
            epoller_.ModFd(client.GetFd(), EPOLLET | EPOLLOUT);
        } else {
            epoller_.ModFd(client.GetFd(), EPOLLET | EPOLLIN);
        }
    }
}

void WebServer::OnWrite(HttpConn &client)
{
    int err = 0;
    int ret = client.Write(err);
    if (ret < 0 && err != EAGAIN) {
        CloseConn(client);
        return;
    }
    epoller_.ModFd(client.GetFd(), EPOLLIN | EPOLLET);
}

void WebServer::Run()
{
    while (true) {
        int num = epoller_.Wait();
        for (int i = 0; i < num; ++i) {
            int events = epoller_.GetEvents(i);
            int fd = epoller_.GetEventFd(i);
            if (fd == listenFd_) {
                HandleNewConn();
            } else if (events & (EPOLLRDHUP & EPOLLHUP & EPOLLERR)) {
                close(fd);
            } else if (events & EPOLLIN) {
                HandleRead(fd);
            } else if (events & EPOLLOUT) {
                HandleWrite(fd);
            } else {

            }
        }
    }
}

bool WebServer::InitSocket()
{
    ::signal(SIGPIPE, SIG_IGN);

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0) {
        return false;
    }

    // 设置地址复用
    int optval = 1;
    int ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (ret < 0) {
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(listenFd_, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (ret < 0) {
        return false;
    }

    ret = listen(listenFd_, 6);
    if (ret < 0) {
        return false;
    }

    if (!epoller_.AddFd(listenFd_, EPOLLIN)) {
        return false;
    }

    return true;
}