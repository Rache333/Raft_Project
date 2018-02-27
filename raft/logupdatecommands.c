//
// Created by ubuntu on 25/02/18.
//
#include "logupdatecommands.h"


int delete_cmd(char * key) {
    if (self.node_state != LEADER) {
        return -1;
    }
    mqd_t mq;
    char buffer[MSG_SIZE];
    /* open the mail queue */
    mq = mq_open("Event queue", O_WRONLY);
    mq_send(mq, buffer, MSG_SIZE, 0);

    return 0;
}


int show_node_status() {
    return (int)(self.node_state);
}
