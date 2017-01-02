//
// Created by hhj on 16-12-31.
//

#include "Epoll.h"
#include <iostream>
#include <thread>

using namespace std;

//int Epoll::LoopFunc(Epoll *pThis);

int Epoll::LoopFunc(Epoll *pThis)
{
    pThis->EpollWait();
    return 0;
}

Epoll::Epoll()
{
    pRecvBuff = new(std::nothrow) char[RecvBuffSize];
    if (pRecvBuff == NULL)
    {
        exit(-1);
    }
    memset(&m_epev, 0, sizeof(epoll_event));
}

Epoll::~Epoll()
{
    if (pRecvBuff)
    {
        delete[] pRecvBuff;
    }
}

int Epoll::EpollInit( EpoolReadFunc ReadFunc)
{
    EpollCreate();
    //EpollAddFd(fd, event, ReadFunc);
    thread td(LoopFunc,this);
    td.detach();
    return 0;
}

int Epoll::EpollAddFd(int fd, int event, EpoolReadFunc ExecFun)
{
    m_epev.data.fd = fd;
    //ev.data.ptr = (void*)ExecFun;
    m_epev.events = event|EPOLLRDHUP;//EPOLLET|EPOLLIN|EPOLLOUT;
    int ret =epoll_ctl(m_epollfd, EPOLL_CTL_ADD, fd, &m_epev);
    return ret;
}

int Epoll::EpollDelFd(int fd, int event)
{
    m_epev.data.fd = fd;
    m_epev.events = event;
    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, fd, &m_epev);
    return 0;
}

int Epoll::EpollModFd(int fd, int event)
{
    m_epev.data.fd = fd;
    m_epev.events = event|EPOLLRDHUP;
    epoll_ctl(m_epollfd, EPOLL_CTL_MOD, fd, &m_epev);
    return 0;
}

int Epoll::EpollCreate()
{
    m_epollfd = epoll_create1(0);
    return 0;
}

int Epoll::EpollWait()
{

    bool flag = true;
    for (; flag;)
    {
        timeval tm={10,0};
        int nfds = 0;
        nfds = epoll_wait(m_epollfd, m_Events, MAX_EVENT, -1);
        switch (nfds)
        {
        case -1:
        {
            if(errno==EINTR)
            {
                cout <<strerror(errno)<<errno<<endl;
                continue;
            }
            cout <<strerror(errno)<<errno<<endl;
            cout << "epoll connect error" << endl;
        }
            break;
        case 0:
        {
            cout <<strerror(errno)<<errno<<endl;
            cout << "recv timeout" << endl;
            /*int n=send(3,"hello",5,0);
            if(n==-1)
            {
                cout <<strerror(errno)<<";timeout ="<<nRet<<" ; fd="<<3<<endl;
            }*/
        }
            break;
        default:
        {
            for (int i = 0; i < nfds; ++i)//假设现在都是作为客户端，收到服务端的消息
            {
                EpoolReadFunc pExecfunc;
                (m_Events+i)->data.ptr == NULL ? pExecfunc = CallbackFunc : pExecfunc = (EpoolReadFunc) (m_Events+i)->data.ptr;
                int RecvFd = (m_Events+i)->data.fd;
                /**********作为客户端，设置非阻塞fd后，connect服务端失败后（1:服务端IP不通，这里会立即收到结果;2:IP通但是端口没打开，
                 * 则会等到一个超时时间大概60s，然后在这里收到结果）,
                 1：LT模式在这里会同时收到EPOLLRDHUP,EPOLLIN,EPOLLERR,EPOLLHUP四种错误,即(m_Events+i)->events==0x2019;
                 2：ET模式
                */

                /*EPOLLRDHUP:对端主动关闭后，会同时收到这个错误和EPOLLIN。即(m_Events+i)->events==0x2001
                 *
                 */
                if((m_Events+i)->events & EPOLLRDHUP)
                {
                    cout <<strerror(errno)<<errno<<endl;
                    cout << " The peer close the fd:"<<RecvFd<<endl;
                    close(RecvFd);
                    break;
                }

                if ((m_Events+i)->events & EPOLLIN)
                {
                    ProcRecv(RecvFd);
                    EpollModFd(RecvFd,WRITE);
                    //close(RecvFd);
                }
                else if ((m_Events+i)->events & EPOLLOUT)
                {
                    for(int i=0;i<100*10;++i)
                    {
                        int nRet = ProcSend(RecvFd);
                        nRet = -1;
                        if (nRet == -1)
                        {
                            break;
                        }
                    }
                    EpollModFd(RecvFd,READ|EPOLLRDHUP);
                }
            }
        }
            break;
        }
    }
    return 0;

}

int Epoll::ProcSend(int RecvFd) const
{
    char SendBuff[1024]={0};
    static uint no=0;
    sprintf(SendBuff,"seq:%d. %s\n",no++,"Send EPOLL OUT");
    //strcpy(SendBuff,"Send EPOLL OUT.");
    int len = strlen(SendBuff);
    int nret = send(RecvFd, SendBuff, len, 0);

    if (nret == -1)
    {
        //EPIPE:当对端已经关闭了，自己还继续用send通过RecvFd发送消息，会在这里得到EPIPE错误。在接下来的epoll_wait中，会收到这个RecvFd的event为EPOLLRDHUP;
        //EAGAIN:自己的发送缓冲区满了，发不动了，会在这里得到EAGAIN错误。
        //cout << strerror(errno) << ";errno =" << errno << " ; fd=" << RecvFd << endl;
        if(errno==EAGAIN)
        {
            cout <<"send EAGAIN.no="<<no<<endl;
            return 0;

        }
        else if (errno==EPIPE)
        {
            cout <<"send errno,errno= EPIPE"<<endl;
            return -1;
        }
        else
        {
            cout << strerror(errno) << ";errno =" << errno << " ; fd=" << RecvFd << endl;
            return -1;
        }
        //close(RecvFd);
    }
    else
    {
        cout <<"no:"<<no<< "; send msg is: " << SendBuff << endl;
    }
}

int Epoll::ProcRecv(int RecvFd) const
{
    int nret = recv(RecvFd, pRecvBuff, RecvBuffSize, 0);
    if (nret == 0)
    {
        cout << "Peer close " << endl;
    }
    else if (nret == -1)
    {
        cout << strerror(errno) << ";errno =" << errno << " ; fd=" << RecvFd << endl;
        close(RecvFd);
    }
    else
    {
        cout << "recv msg is: " << pRecvBuff << endl;
    }
}