#ifndef RAFT_LIBRARY_H
#define RAFT_LIBRARY_H
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>

#define NODE_N 4
#define MAJORITY 3
#define GEN_ELECTION_TIMEOUT (((150) + ((rand()) % (150))) * (1000))

typedef enum states {
    FOLLOWER = 0,
    CANDIDATE,
    LEADER
} state_t;

typedef enum msg_types {
    HEART_BEAT = 0,
    ADD,
    EDIT,
    DELETE,
    COMMIT
} msg_type_t;

typedef enum events {
    /* Election related events */
    ELECTION_TIMEOUT = 0,
    VOTE_REQUEST,
    VOTE,
    /* Appendentries related events */
    HEARTBEAT_APPENDENTRIES_MSG,
    ADD_APPENDENTRIES_MSG,
    EDIT_APPENDENTRIES_MSG,
    DELETE_APPENDENTRIES_MSG,
    COMMIT_APPENDENTRIES_MSG,
    APPENDENTRIES_MSG_ACK,
    HEARTBEAT_TIMEOUT
} event_t;

typedef struct node_modes {
    state_t node_state;
    int term;
    int election_timeout;
    int vote_count;
} node_mode_t;

typedef struct election_vote_reqs {
    int candidate_id;
    int term;
} election_vote_req_t;

typedef struct election_votes {
    int term;
} election_vote_t;

typedef struct appendentries_msgs {
    int term;
    int msg_id;
    msg_type_t msg_type;
    char key[50];
    char data[50];
} appendentries_msg_t;

typedef struct appendentries_msg_acks {
    int term;
    int msg_id;
} appendentries_msg_ack_t;

void init();
void send_vote_req(int id);
void send_appendentries(struct appendentries_msg);
void commit(struct appendentries_msg, int accepted);
#endif
