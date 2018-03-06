#ifndef RAFT_LIBRARY_H
#define RAFT_LIBRARY_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
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
#include <time.h>
#include <ifaddrs.h>

#include "log_update_commands.h"
#include "multicast_handling.h"
#include "log_replication.h"
#include "election.h"

#define NODE_N 4
#define MAJORITY 3
#define GEN_ELECTION_TIMER (((150) + ((rand()) % (150))) * (6000)) //10 times greater timer for debugging.
#define STATE_N ((LEADER) + (1))
#define EVENT_N ((LOG_UPDATE) + (1))
#define MSG_TYPES_N ((COMMIT) + (1))
#define UPDATE_TYPES_N ((UPDATE_DELETE) + (1))
#define MSG_SIZE 1024
#define BFRSIZE 512
#define RAFT_MSG_SIZE 10
//#define MY_IP "100.64.1.175"
#define EVENT_QUEUE "/Event queue2"
#define NODE_ID 1


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
    int term;
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

void * run(void *);
void node_init();
void * listen_to_msgs(void *);


void send_heartbeat(char * cmd);

void log_update_hndlr(char * cmd);

void update_add_hndlr(char * p);
void update_edit_hndlr(char * p);
void update_delete_hndlr(char * p);

void node_stepdown();

#endif
