//
// Created by ubuntu on 25/02/18.
//

#ifndef MULTICAST_HNDLR_H
#define MULTICAST_HNDLR_H


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 12345
#define MULTI_GROUP "225.0.0.37"
#define MAX_BUF_SIZE 256

int init_multicast_sender(struct sockaddr_in * addr);

int init_multicast_receiver(struct sockaddr_in * addr);

void send_msg(int fd, char * msg, struct sockaddr_in * addr);

void recv_msg(int fd, struct sockaddr_in * addr, char * recv_buff, int len);

#endif //MULTICAST_HNDLR_H
