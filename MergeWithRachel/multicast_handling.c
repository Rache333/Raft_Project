
#include "multicast_handling.h"

/////// NEW FILE FROM RACHEL ///////////

int init_multicast_sender(struct sockaddr_in * addr)
{
    int sender_fd;

    /* create the sender UDP socket */
    if ((sender_fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        perror("failed create sender UDP socket");
        exit(1);
    }

    /* set up destination address to multi-group */
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr(MULTI_GROUP);
    addr->sin_port = htons(PORT);

    return sender_fd;
}

int init_multicast_listener(struct sockaddr_in * addr)
{

    int listener_fd;
    u_int yes = 1;
    struct ip_mreq mreq;

    /* create the listener UDP socket */
    if ((listener_fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
        perror("failed create listener UDP socket");
        exit(1);
    }

    /* allow multiple sockets to use the same PORT number */
    if (setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("Reusing ADDR failed");
        exit(1);
    }

    /* set up destination address to any */
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = htons(PORT);

    /* bind to receive address */
    if (bind(listener_fd, (struct sockaddr *) addr,sizeof(*addr)) < 0) {
        perror("bind listener failed");
        exit(1);
    }

    /* use setsockopt() to request the kernel to join a multicast group */
    mreq.imr_multiaddr.s_addr = inet_addr(MULTI_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(listener_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt for multicast failed");
        exit(1);
    }

    return listener_fd;
}

void send_msg(int fd, char * msg, struct sockaddr_in * addr)
{
    if (sendto(fd, msg, strlen(msg),0,(struct sockaddr *) addr,
               sizeof(*addr)) < 0) {
        perror("send msg failed");
        exit(1);
    }
}

void recv_msg(int fd, struct sockaddr_in * addr, char * recv_buff, int len)
{
    int nbytes;
    int addrlen = sizeof(*addr);
    if ((nbytes = recvfrom(fd, recv_buff, len ,0,(struct sockaddr *) addr, &addrlen)) < 0) {
        perror("recv msg failed");
        exit(1);
    }
}