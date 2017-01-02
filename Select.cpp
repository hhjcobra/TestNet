#include <iostream>
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

using namespace std;
#define ANET_ERR -1
#define ANET_OK 0
#define ANET_CONNECT_NONBLOCK 1
typedef set<int> T_SET_FD;


static int anetSetBlock(int fd, int non_block) {
    int flags;

    /* Set the socket blocking (if non_block is zero) or non-blocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1)
    {
        return ANET_ERR;
    }

    if (non_block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        return ANET_ERR;
    }
    return ANET_OK;
}

static int anetSetReuseAddr( int fd) {
    int yes = 1;
    /* Make sure connection-intensive things like the redis benckmark
     * will be able to close/open sockets a zillion of times */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        return ANET_ERR;
    }
    return ANET_OK;
}

int selectmain()
{
    T_SET_FD SET_FD;
    uint16_t serverport = 8886;
    uint16_t InitPort =9000;
    string serverip = "192.168.1.68";
    string localip = "192.168.1.198";
    sockaddr_in bindaddr,serveraddr;
    inet_pton(AF_INET,serverip.c_str(),&serveraddr.sin_addr);


    //serveraddr.sin_addr.s_addr = inet_addr(serverip.c_str());
    serveraddr.sin_port = htons(serverport);
    serveraddr.sin_family=AF_INET;

    bindaddr.sin_family=AF_INET;
    inet_pton(AF_INET,localip.c_str(),&bindaddr.sin_addr);
    for(int i = 0;i<5;++i)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd != -1)
        {
            anetSetReuseAddr(fd);
            SET_FD.insert(fd);
            bindaddr.sin_port = htons(InitPort+i);
            anetSetBlock(fd, 1);
            int nret = bind(fd, (sockaddr *) &bindaddr, sizeof(sockaddr));
            if (connect(fd, (sockaddr *) &serveraddr, sizeof(sockaddr)) == -1)
            {
                /* If the socket is non-blocking, it is ok for connect() to
                 * return an EINPROGRESS error here. */
                if (errno == EINPROGRESS  && ANET_CONNECT_NONBLOCK)
                {

                }
                cout <<strerror(errno)<<errno<<endl;
                    continue;
                close(fd);
                cout << "connect error"<<endl;
                continue;
            }
        }
    }
    fd_set fds;
    char buffer[101];
    bool loop=true;
    while(loop)
    {
        FD_ZERO(&fds);
        if(SET_FD.size()==0)
        {
            break;
        }
        for (T_SET_FD::iterator iter=SET_FD.begin();iter !=SET_FD.end();++iter)
        {
            FD_SET(*iter,&fds);
        }
        int k=*SET_FD.rbegin()+1;
        timeval tm={5,0};
        int nRet = select(k,&fds,NULL,NULL,&tm);
        switch (nRet)
        {
        case -1:
        {
            cout << "connect error" << endl;
        }
            break;
        case 0:
        {
            cout << "recv timeout" << endl;
            int n=send(3,"hello",5,0);
            if(n==-1)
            {
                cout <<strerror(errno)<<";timeout ="<<nRet<<" ; fd="<<3<<endl;
            }
        }
            break;
        default:
        {
            for(T_SET_FD::iterator it=SET_FD.begin();it!=SET_FD.end();)
            {
                if (FD_ISSET(*it, &fds))
                {
                    int len=recv(*it, buffer, 100, 0);
                    if(len == -1)
                    {
                        cout <<strerror(errno)<<";nRet ="<<nRet<<" ; fd="<<*it<<endl;
                        close(*it);
                        SET_FD.erase(it++);
                        continue;
                    }
                    else if (len == 0)
                    {
                        cout <<"closed;nRet ="<<nRet<<" ; fd="<<*it<<endl;
                        close(*it);
                        SET_FD.erase(it++);
                        continue;
                    }
                    cout << buffer << endl;
                }
                ++it;
            }
        }
            break;
        }




    }




    std::cout << "Hello, World!" << std::endl;
    return 0;
}