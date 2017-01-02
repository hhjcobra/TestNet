//
// Created by hhj on 16-12-31.
//

#ifndef TESTNET_EPOLL_H
#define TESTNET_EPOLL_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <set>
#include <string>
#include <fcntl.h>
#include <cstring>
//#include <cerrno>
#include <cstdint>
#include <errno.h>
#include <sys/epoll.h>

using namespace std;

enum EPOLL_EVENT_OPT
{
    READ=EPOLLIN,
    WRITE=EPOLLOUT,
    ET=EPOLLET,
};

typedef void (*EpoolReadFunc)(int flag);
class Epoll
{
public:
    Epoll();
    ~Epoll();
    int EpollInit(EpoolReadFunc ReadFunc);
    int EpollAddFd(int fd,int event,EpoolReadFunc ExecFun=NULL);
    int EpollDelFd(int fd,int event);
    int EpollModFd(int fd,int event);
    int EpollCreate();
    int EpollWait();

private:
    static int LoopFunc(Epoll *pThis);
private:
    static const int  MAX_EVENT=1024;
    static const int  RecvBuffSize=5*1024;
    epoll_event m_epev;
    int m_epollfd;
    epoll_event m_Events[MAX_EVENT];
    EpoolReadFunc CallbackFunc;
    char *pRecvBuff;

    int ProcRecv(int RecvFd) const;

    int ProcSend(int RecvFd) const;
};


#endif //TESTNET_EPOLL_H
