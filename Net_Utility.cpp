//
// Created by hhj on 16-12-31.
//

#include "Net_Utility.h"

int anetSetBlock(int fd, int non_block)
{
    int flags;

    /* Set the socket blocking (if non_block is zero) or non-blocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1)
    {
        return ANET_ERR;
    }

    if (non_block)
    {
        flags |= O_NONBLOCK;
    }
    else
    {
        flags &= ~O_NONBLOCK;
    }

    if (fcntl(fd, F_SETFL, flags) == -1)
    {
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetSetReuseAddr(int fd)
{
    int yes = 1;
    /* Make sure connection-intensive things like the redis benckmark
     * will be able to close/open sockets a zillion of times */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    {
        return ANET_ERR;
    }
    return ANET_OK;
}