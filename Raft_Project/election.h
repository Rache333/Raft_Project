
#ifndef RAFT_ELECTION_H
#define RAFT_ELECTION_H

#include "raft.h"

void election_timeout_hndlr(char * cmd);

void vote_req_hndlr(char * cmd);
void l_vote_req_hndlr(char * cmd);

void vote_hndlr(char * cmd);


#endif //RAFT_ELECTION_H
