//
// Created by ubuntu on 25/02/18.
//

#ifndef RAFT_LOGUPDATECOMMANDS_H
#define RAFT_LOGUPDATECOMMANDS_H
#include "raft.h"

typedef void (* delete_callback_t)(char * key);
typedef void (* edit_callback_t)(char * key, char * value);

int delete_cmd(char * key);
int edit_cmd(char * key, char * value);
int show_node_status();
int show_system_status();


#endif //RAFT_LOGUPDATECOMMANDS_H
