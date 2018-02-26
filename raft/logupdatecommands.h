//
// Created by ubuntu on 25/02/18.
//

#ifndef RAFT_LOGUPDATECOMMANDS_H
#define RAFT_LOGUPDATECOMMANDS_H
#include "raft.h"

int delete_cmd(char * key);
int edit_cmd(char * key, char * value);

#endif //RAFT_LOGUPDATECOMMANDS_H
