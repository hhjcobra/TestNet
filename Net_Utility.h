//
// Created by hhj on 16-12-31.
//

#ifndef TESTNET_NETUTILITY_H
#define TESTNET_NETUTILITY_H
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



#define ANET_ERR -1
#define ANET_OK 0
#define ANET_CONNECT_NONBLOCK 1

int anetSetBlock(int fd, int non_block);
int anetSetReuseAddr( int fd);

#endif