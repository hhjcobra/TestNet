#include "Epoll.h"
#include "Net_Utility.h"
#include <cstdint>
#include <errno.h>
#include <iostream>
#include <thread>

void Callback(int arg)
{

}

void ThreadFunc(int fd)
{
    sleep(300);
    cout <<"CLOSE FD 88******************"<<endl;
    close(fd);
    exit(0);
}

int main()
{

    uint16_t serverport = 8887;
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
    Epoll myEpoll;
    myEpoll.EpollInit(Callback);
    //sleep(3);
    int testfd;
    for(int i = 0;i<1;++i)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);

        if (fd != -1)
        {
            testfd =fd;
            socklen_t getnumlen=4;
            int buflen=0;
            buflen=4*1024;
            if(0!=setsockopt(fd,SOL_SOCKET,SO_RCVBUF,&buflen,sizeof(int)))
            {
                return 0;
            }
            buflen=1024;
            if(0!=setsockopt(fd,SOL_SOCKET,SO_SNDBUF,&buflen,sizeof(int)))
            {
                return 0;
            }
            //读取写缓存大小
            if(0!=getsockopt(fd,SOL_SOCKET,SO_RCVBUF,&buflen,&getnumlen))
            {
                printf("\n%s,%d\n",strerror(errno),errno);
                return 0;
            }
            if(0!=getsockopt(fd,SOL_SOCKET,SO_SNDBUF,&buflen,&getnumlen))
            {
                printf("\n%s,%d\n",strerror(errno),errno);
                return 0;
            }

            anetSetReuseAddr(fd);
            bindaddr.sin_port = htons(InitPort+i);
            anetSetBlock(fd, 1);

            //int nret = bind(fd, (sockaddr *) &bindaddr, sizeof(sockaddr));
            if (connect(fd, (sockaddr *) &serveraddr, sizeof(sockaddr)) == -1)
            {
                /* If the socket is non-blocking, it is ok for connect() to
                 * return an EINPROGRESS error here. */
                if (errno == EINPROGRESS  && ANET_CONNECT_NONBLOCK)
                {
                    myEpoll.EpollAddFd(fd,WRITE);
                    cout <<"main 82: test."<<endl;
                }
                cout <<strerror(errno)<<errno<<endl;
                continue;
                close(fd);
                cout << "connect error"<<endl;
                continue;
            }


            myEpoll.EpollAddFd(fd,READ);

        }
    }
    thread td(ThreadFunc,testfd);
    td.detach();

    while(1)
    {
        static int count =0;
        /*static int no =0;
        char SendBuff[1024]={0};
        sprintf(SendBuff,"seq:%d. %s\n",no++,"Send EPOLL OUT");
        //strcpy(SendBuff,"Send EPOLL OUT.");
        int len = strlen(SendBuff);
        int nret = send(testfd, SendBuff, len, 0);
        if(nret<=0)
        {
            cout <<"main 107:";
            cout << strerror(errno) << ";errno =" << errno << " ; fd=" << testfd << endl;
        }*/

        sleep(1);
        cout <<"main 111: count=."<<++count<<endl;
        //myEpoll.EpollModFd(testfd,WRITE);

    }
    return 0;
}