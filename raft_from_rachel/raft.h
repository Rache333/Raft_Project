#ifndef RAFT_LIBRARY_H
#define RAFT_LIBRARY_H

#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/socket.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

#define NODE_N 4
#define MAJORITY 3
#define GEN_ELECTION_TIMER (((150) + ((rand()) % (150))) * (1000))
#define STATE_N ((LEADER) + (1))
#define EVENT_N ((LOG_UPDATE) + (1))

#define RAFT_MSG_SIZE 10

typedef enum states {
    FOLLOWER = 0,
    CANDIDATE,
    LEADER
} state_t;


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

typedef struct appendentries_msgs {
    int msg_term;
    int msg_src_ip;
    msg_type_t msg_type;
    char key[50];
    char value[50];
} appendentries_msg_t;

typedef struct log_entries
{
    int term;
    msg_type_t type;
    char key[50];
    char value[50];
    int has_commited;
} log_entry_t;

typedef struct node_modes {
    state_t node_state;
    unsigned int term;
    int election_timer_interval;

    int vote_count;

    /* for ipc udp multi-cast rafts communication */
    int sender_sock_fd;
    struct sockaddr_in s_addr;
    int listener_sock_fd;
    struct sockaddr_in l_addr;

    /* to maintain log */
    log_entry_t log[1000];
    int log_len;
    int log_num_of_committed;

} node_mode_t;

typedef void (*event_handler_t)(void * p);

static node_mode_t self;

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


void init();
void node_init();







void commit(struct appendentries_msg, int accepted);

#endif //RAFT_LIBRARY_H
