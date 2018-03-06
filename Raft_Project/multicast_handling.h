
#ifndef MULTICAST_HNDLR_H
#define MULTICAST_HNDLR_H

#include <arpa/inet.h>

#include "raft.h"

#define PORT 62345
#define MULTI_GROUP "225.0.0.38"

int init_multicast_sender(struct sockaddr_in * addr);

int init_multicast_listener(struct sockaddr_in * addr);

void send_msg(int fd, char * msg, struct sockaddr_in * addr);

void recv_msg(int fd, struct sockaddr_in* addr, char* recv_buf, char* s_addr , int len);


#endif //MULTICAST_HNDLR_H