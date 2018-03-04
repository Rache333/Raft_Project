
#ifndef RAFT_LOG_REPLICATION_H
#define RAFT_LOG_REPLICATION_H

void follower_msg_hndlr(char * msg);
void candidate_msg_hndlr(char * msg);
void leader_msg_hndlr(char * msg);

typedef void (*event_handler_t)(char *);

static event_handler_t event_handlers[STATE_N][EVENT_N] = {
        [FOLLOWER][ELECTION_TIMEOUT] = election_timeout_hndlr,
        [CANDIDATE][ELECTION_TIMEOUT] = election_timeout_hndlr,

        [FOLLOWER][VOTE_REQUEST] = vote_req_hndlr,
        [CANDIDATE][VOTE_REQUEST] = vote_req_hndlr,
        [LEADER][VOTE_REQUEST] = l_vote_req_hndlr,

        [FOLLOWER][APPENDENTRIES_MSG] = follower_msg_hndlr,
        [CANDIDATE][APPENDENTRIES_MSG] = candidate_msg_hndlr,
        [LEADER][APPENDENTRIES_MSG] = leader_msg_hndlr,

        [LEADER][LOG_UPDATE] = log_update_hndlr
};

int heartbeat_msg_hndlr(msg_t mt);
int add_msg_hndlr(msg_t mt);
int edit_msg_hndlr(msg_t mt);
int delete_msg_hndlr(msg_t mt);
int commit_msg_hndlr(msg_t mt);


static event_handler_t msg_handlers[MSG_TYPES_N] = {

        [HEART_BEAT] = heartbeat_msg_hndlr,
        [ADD] = add_msg_hndlr,
        [EDIT] = edit_msg_hndlr,
        [DELETE] = delete_msg_hndlr,
        [COMMIT] = commit_msg_hndlr
};



typedef struct msg {
    int term;
    char key[50];
    char value[50];
    char src_ip[50];

} msg_t;


#endif //RAFT_LOG_REPLICATION_H
