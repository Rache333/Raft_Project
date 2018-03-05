#ifndef RAFT_LIBRARY_H
#define RAFT_LIBRARY_H
#include "logreplication.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "election.h"
#include "logupdatecommands.h"
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include "multicast_handling.h"
//#include "logreplication.h"


#define NODE_N 4
#define MAJORITY 3
#define GEN_ELECTION_TIMER (((150) + ((rand()) % (150))) * (1000))
#define STATE_N ((LEADER) + (1))
#define EVENT_N ((LOG_UPDATE) + (1))
#define MSG_TYPES_N ((COMMIT) + (1))
#define UPDATE_TYPES_N ((UPDATE_DELETE) + (1))
#define MSG_SIZE 1024
#define BFRSIZE 512
#define RAFT_MSG_SIZE 10

typedef struct msg {
    int term;
    char key[50];
    char value[50];
    char src_ip[50];

} msg_t;

void follower_msg_hndlr(char * msg);
void candidate_msg_hndlr(char * msg);
void leader_msg_hndlr(char * msg);
void msg_ack_hndlr(char * msg);

typedef int (* msg_handler_t)(msg_t);

int heartbeat_msg_hndlr(msg_t mt);
int add_msg_hndlr(msg_t mt);
int edit_msg_hndlr(msg_t mt);
int delete_msg_hndlr(msg_t mt);
int commit_msg_hndlr(msg_t mt);


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
    HEARTBEAT,
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
    unsigned char log_ack_count[1000];

} node_mode_t;

typedef void (*event_handler_t)(char *);

typedef struct election_vote_reqs {
    int candidate_id;
    int term;
} election_vote_req_t;

typedef struct election_votes {
    int term;
} election_vote_t;

typedef struct appendentries_msgs {
    int msg_term;
    int msg_src_ip;
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

node_mode_t self;
void run(void *);
void node_init();
void listen_to_msgs();
void send_appendentries(struct appendentries_msgs);
void commit(struct appendentries_msgs, int accepted);

void election_timeout_hndlr(char * cmd);
void vote_req_hndlr(char * p);
void l_vote_req_hndlr(char * p);
void log_update_hndlr(char * p);

void f_append_entry_msg_hndlr(char * p);
void c_append_entry_msg_hndlr(char * p);
void l_append_entry_msg_hndlr(char * p);

void update_add_hndlr(char * p);
void update_edit_hndlr(char * p);
void update_delete_hndlr(char * p);


static event_handler_t event_handlers[STATE_N][EVENT_N] = {
        [FOLLOWER][ELECTION_TIMEOUT] = election_timeout_hndlr,
        [CANDIDATE][ELECTION_TIMEOUT] = election_timeout_hndlr,

        [FOLLOWER][VOTE_REQUEST] = vote_req_hndlr,
        [CANDIDATE][VOTE_REQUEST] = vote_req_hndlr,
        [LEADER][VOTE_REQUEST] = l_vote_req_hndlr,

        [FOLLOWER][APPENDENTRIES_MSG] = follower_msg_hndlr,
        [CANDIDATE][APPENDENTRIES_MSG] = candidate_msg_hndlr,
        [LEADER][APPENDENTRIES_MSG] = leader_msg_hndlr,

        [LEADER][APPENDENTRIES_MSG_ACK] = msg_ack_hndlr,

        [LEADER][LOG_UPDATE] = log_update_hndlr
};

static event_handler_t log_update_handlers[UPDATE_TYPES_N] = {
        [UPDATE_ADD] = update_add_hndlr,
        [UPDATE_EDIT] = update_edit_hndlr,
        [UPDATE_DELETE] = update_delete_hndlr
};


static const msg_handler_t msg_handlers[MSG_TYPES_N] = {

        [HEARTBEAT] = heartbeat_msg_hndlr,
        [1] = add_msg_hndlr,
        [2] = edit_msg_hndlr,
        [3] = delete_msg_hndlr,
        [4] = commit_msg_hndlr
};


#endif
