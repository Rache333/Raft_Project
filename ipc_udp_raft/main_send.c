#include <stdio.h>

#include "multicast_handling.h"

int main() {
    printf("Multicast Lab sender!\n");


    struct sockaddr_in addr;
    int sender_sock_fd = init_multicast_sender(&addr);
    puts("after sender init ip addrs :");
    puts(inet_ntoa(addr.sin_addr));

    send_msg(sender_sock_fd, "vote for me!!! :D", &addr);
    puts("after sender send msg ip addrs :");
    puts(inet_ntoa(addr.sin_addr));


    char recv_buff1[MAX_BUF_SIZE];
    recv_msg(sender_sock_fd, &addr, recv_buff1, MAX_BUF_SIZE);
    puts(recv_buff1);
    puts("after sender receive msg ip addrs :");
    puts(inet_ntoa(addr.sin_addr));


    return 0;
}