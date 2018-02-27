#include <stdio.h>

#include "multicast_handling.h"

int main() {
    printf("Multicast Lab receiver!\n");


    struct sockaddr_in addr;
    int listener_sock_fd = init_multicast_receiver(&addr);

    puts("after receiver init ip addrs :");
    puts(inet_ntoa(addr.sin_addr));

    char recv_buff2[MAX_BUF_SIZE];
    recv_msg(listener_sock_fd, &addr, recv_buff2, MAX_BUF_SIZE);
    puts(recv_buff2);
    puts("after receiver recv msg ip addrs :");
    puts(inet_ntoa(addr.sin_addr));

    send_msg(listener_sock_fd, "u got my vote!!! :D", &addr);
    puts("after receiver send msg ip addrs :");
    puts(inet_ntoa(addr.sin_addr));

    return 0;
}