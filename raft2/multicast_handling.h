 ///// NEW FILE FROM RACHEL ///////

#ifndef MULTICAST_HNDLR_H
#define MULTICAST_HNDLR_H


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "raft.h"

#define PORT 62345
#define MULTI_GROUP "225.0.0.38"

int init_multicast_sender(struct sockaddr_in * addr);

int init_multicast_listener(struct sockaddr_in * addr);

void send_msg(int fd, char * msg, struct sockaddr_in * addr);

void recv_msg(int fd, struct sockaddr_in* addr, char* recv_buf, char* s_addr , int len);


#endif //MULTICAST_HNDLR_H