
#ifndef RAFT_LOG_REPLICATION_H
#define RAFT_LOG_REPLICATION_H

#include "raft.h"

typedef struct msg {
    int term;
    char key[50];
    char value[50];
    char src_ip[50];

} msg_t;

typedef int (* msg_handler_t)(msg_t);

void follower_msg_hndlr(char * msg);
void candidate_msg_hndlr(char * msg);
void leader_msg_hndlr(char * msg);

void msg_ack_hndlr(char * msg);

int heartbeat_msg_hndlr(msg_t mt);
int add_msg_hndlr(msg_t mt);
int edit_msg_hndlr(msg_t mt);
int delete_msg_hndlr(msg_t mt);
int commit_msg_hndlr(msg_t mt);


#endif // RAFT_LOG_REPLICATION_H
