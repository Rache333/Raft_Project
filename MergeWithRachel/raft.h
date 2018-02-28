#ifndef RAFT_LIBRARY_H
#define RAFT_LIBRARY_H
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "election.h"
#include "logreplication.h"
#include "logupdatecommands.h"
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>


#define NODE_N 4
#define MAJORITY 3
#define GEN_ELECTION_TIMER (((150) + ((rand()) % (150))) * (1000))
#define STATE_N ((LEADER) + (1))
#define EVENT_N ((LOG_UPDATE) + (1))
#define MSG_TYPES_N ((COMMIT) + (1))
#define MSG_SIZE 1024
#define BFRSIZE 512

typedef void (* delete_callback_t)(char * key);
typedef void (* edit_callback_t)(char * key, char * value);

typedef enum states {
    FOLLOWER = 0,
    CANDIDATE,
    LEADER
} state_t;

typedef enum update_types {
    UPDATE_ADD = 0,
    UPDATE_EDIT,
    UPDATE_DELETE
} update_type_t;

typedef enum msg_types {
    ADD = 0,
    EDIT,
    DELETE,
    HEART_BEAT,
    COMMIT
} msg_type_t;

typedef enum events {
    /* Election related events */
    ELECTION_TIMEOUT = 0,
    VOTE_REQUEST,
    VOTE,
    /* Appendentries related events */
    APPENDENTRIES_MSG,
    APPENDENTRIES_MSG_ACK,
    HEARTBEAT_TIMEOUT,
    LOG_UPDATE
} event_t;

typedef struct node_modes {
    state_t node_state;
    int term;
    int election_timer_interval;
    int vote_count;
} node_mode_t;

typedef void (*event_handler_t)(void *);

typedef struct election_vote_reqs {
    int candidate_id;
    int term;
} election_vote_req_t;

typedef struct election_votes {
    int term;
} election_vote_t;

typedef struct appendentries_msgs {
    int msg_term;
    int msg_id;
    msg_type_t msg_type;
    char key[50];
    char value[50];
} appendentries_msg_t;

typedef struct appendentries_msg_acks {
    int msg_term;
    int msg_id;
    int my_term;
} appendentries_msg_ack_t;

typedef struct callback_types {
    delete_callback_t _delete;
    edit_callback_t _edit;
} callback_types_t;

//////////// FROM RACHEL ////////////


void election_timeout_hndlr(void * p);
void vote_req_hndlr(void * p);
void l_vote_req_hndlr(void * p);
void log_update_hndlr(void * p);

void f_append_entry_msg_hndlr(void * p);
void c_append_entry_msg_hndlr(void * p);
void l_append_entry_msg_hndlr(void * p);

static event_handler_t event_handlers[STATE_N][EVENT_N] = {
        [FOLLOWER][ELECTION_TIMEOUT] = election_timeout_hndlr,
        [CANDIDATE][ELECTION_TIMEOUT] = election_timeout_hndlr,

        [FOLLOWER][VOTE_REQUEST] = vote_req_hndlr,
        [CANDIDATE][VOTE_REQUEST] = vote_req_hndlr,
        [LEADER][VOTE_REQUEST] = l_vote_req_hndlr,

        [FOLLOWER][APPENDENTRIES_MSG] = f_append_entry_msg_hndlr,
        [CANDIDATE][APPENDENTRIES_MSG] = c_append_entry_msg_hndlr,
        [LEADER][APPENDENTRIES_MSG] = l_append_entry_msg_hndlr,

        [LEADER][LOG_UPDATE] = log_update_hndlr
};

typedef struct log_entries
{
    int term;
    msg_type_t type;
    char key[50];
    char value[50];
    int has_commited;
} log_entry_t;

void commit(struct appendentries_msg, int accepted);

//////////// END FROM RACHEL /////////////


node_mode_t self;
void run(void *);
void node_init();
void join_multicast();
state_t log_update_handler(node_mode_t *);
void send_appendentries(struct appendentries_msg);
void commit(struct appendentries_msg, int accepted);
#endif
