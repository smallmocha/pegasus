#include "webserver.h"
#include <signal.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include "log/log.h"

WebServer::WebServer(uint16_t port, int threadNum) : port_(port), listenFd_(-1), isClosed_(false), threadPool_(std::make_shared<ThreadPool>())
{
    assert(threadPool_);
    threadPool_->Init(threadNum);
    if (!InitSocket()) {
        isClosed_ = true;
    }
    Log *log = Log::Instance();
    log->Init(0);
    char pathBuff[256];
    char *dir = getcwd(pathBuff, sizeof(pathBuff));
    assert(dir != nullptr);
    std::string srcDir(dir);
    srcDir += "/resources/";
    LOG_INFO("dir=%s", srcDir.c_str());
    HttpConn::srcDir_ = srcDir;
}

WebServer::~WebServer()
{
    isClosed_ = true;
    close(listenFd_);
}

void WebServer::UpdateTimer(int fd)
{
    timerQueue_.Del(users_[fd].GetTimer());
    Timer *timer = timerQueue_.Add(fd, 1000 * 20, std::bind(&WebServer::CloseConn, this, std::ref(users_[fd])));
    users_[fd].SetTimer(timer);
}

void WebServer::HandleNewConn()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int connFd = accept(listenFd_, reinterpret_cast<sockaddr *>(&addr), &len);
    if (connFd < 0) {
        return;
    }
    users_[connFd].Init(connFd, addr);
    epoller_.AddFd(connFd, EPOLLIN | EPOLLET | EPOLLONESHOT);
    SetFdNonBlock(connFd);
    Timer *timer = timerQueue_.Add(connFd, 1000 * 20, std::bind(&WebServer::CloseConn, this, std::ref(users_[connFd])));
    users_[connFd].SetTimer(timer);
}

void WebServer::HandleRead(int fd)
{
    assert(users_.count(fd) > 0);
    UpdateTimer(fd);
    threadPool_->AddTask(std::bind(&WebServer::OnRead, this, std::ref(users_[fd])));
}

void WebServer::HandleWrite(int fd)
{
    assert(users_.count(fd) > 0);
    UpdateTimer(fd);
    threadPool_->AddTask(std::bind(&WebServer::OnWrite, this, std::ref(users_[fd])));
}

void WebServer::CloseConn(HttpConn &client)
{
    LOG_INFO("Client[%d] quit!", client.GetFd());
    timerQueue_.Del(client.GetTimer());
    epoller_.DelFd(client.GetFd());
    client.Close();
}

void WebServer::OnProcess(HttpConn &client) {
    if (client.Process()) {
        epoller_.ModFd(client.GetFd(), EPOLLET | EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP);
    } else {
        epoller_.ModFd(client.GetFd(), EPOLLET | EPOLLIN | EPOLLONESHOT | EPOLLRDHUP);
    }
}

void WebServer::OnRead(HttpConn &client)
{
    int err = 0;
    int ret = client.Read(err);
    if (ret <= 0 && err != EAGAIN) {
        LOG_WARNING("fd=%d read err, err=%d", client.GetFd(), err);
        CloseConn(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnWrite(HttpConn &client)
{
    int err = 0;
    int ret = client.Write(err);
    if (client.ToWriteBytes() == 0) {
        // ????????????
        OnProcess(client);
        return;
    }
    if (ret < 0) {
        if (err == EAGAIN) {
            epoller_.ModFd(client.GetFd(), EPOLLET | EPOLLOUT | EPOLLONESHOT | EPOLLRDHUP);
            return;
        }
    }
    CloseConn(client);
}

void WebServer::Run()
{
    while (true) {
        int timeout = timerQueue_.GetNextTick();
        int num = epoller_.Wait(timeout);
        for (int i = 0; i < num; ++i) {
            int events = epoller_.GetEvents(i);
            int fd = epoller_.GetEventFd(i);
            if (fd == listenFd_) {
                HandleNewConn();
            } else if (events & (EPOLLRDHUP & EPOLLHUP & EPOLLERR)) {
                CloseConn(users_[fd]);
            } else if (events & EPOLLIN) {
                HandleRead(fd);
            } else if (events & EPOLLOUT) {
                HandleWrite(fd);
            } else {
                LOG_WARNING("unexpected event");
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

    // ??????????????????
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

    ret = listen(listenFd_, 128);
    if (ret < 0) {
        return false;
    }

    if (!epoller_.AddFd(listenFd_, EPOLLIN | EPOLLRDHUP)) {
        return false;
    }

    return true;
}