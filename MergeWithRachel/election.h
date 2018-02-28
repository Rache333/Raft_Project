//
// Created by ubuntu on 25/02/18.
//

#ifndef RAFT_ELECTION_H
#define RAFT_ELECTION_H
#include "raft.h"

void cast_vote_requests(int id);
void send_vote(void);
void reset_election_timeout(void);


#endif //RAFT_ELECTION_H
