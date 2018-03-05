//
// Created by ubuntu on 25/02/18.
//
#include "logupdatecommands.h"


int delete_cmd(char * key) {

    if (self.node_state != LEADER) {
        return -1;
    }

    mqd_t mq;
    char * str;
    sprintf(str, "%d,%d,%s%c",(int)LOG_UPDATE, (int)UPDATE_DELETE, key, '\0');
    /* open the mail queue */
    mq = mq_open(EVENT_QUEUE, O_WRONLY);
    mq_send(mq, str, sizeof(str), 0);

    return 0;

}

int edit_cmd(char* key, char* value) {

    if (self.node_state != LEADER) {
        return -1;
    }

    mqd_t mq;
    char * str;
    sprintf(str, "%d,%d,%s,%s%c",(int)LOG_UPDATE, (int)UPDATE_EDIT, key, value, '\0');
    /* open the mail queue */
    mq = mq_open(EVENT_QUEUE, O_WRONLY);
    mq_send(mq, str, sizeof(str), 0);

    return 0;
}

int show_node_status() {
    return (int)(self.node_state);
}

int show_system_status() {
    puts("From C: show sys status");
}

void show_log() {
    puts("From C: showing log");
}