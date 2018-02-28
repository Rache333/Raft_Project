#include "raft.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

static pthread_t t1;

static event_handler_t event_handlers[STATE_N][EVENT_N] = {
        [LEADER][LOG_UPDATE] = log_update_handler
};

callback_types_t callback_type;


void run(void * a) {


//    printf("hello from c proc num: %d\n", getpid());
//    fflush(stdout);
//    printf("%d, %d\n", (int)callback_type._delete, (int)callback_type._edit);
//    join_multicast();

    time_t t;
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_curmsgs = 0;
    srand((unsigned)time(&t));
    node_init(& self);
    mqd_t mq;
    mq = mq_open("/Event queue", O_CREAT | O_RDONLY, 0644, &attr);
    char buffer[MSG_SIZE];

    while(1) {

        ssize_t bytes_read;

        /* receive the message */
        bytes_read = mq_receive(mq, buffer, MSG_SIZE, NULL);
        if(bytes_read == -1) {
            printf("%s", strerror(errno));
        }
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);
    }
}

void init(delete_callback_t _delete1, edit_callback_t _edit1) {

    callback_type._delete = _delete1;
    callback_type._edit = _edit1;
    pthread_create(&t1,NULL, &run,NULL);

}


void node_init() {

    self.node_state = FOLLOWER;
    self.term = -1;
    self.election_timer_interval = GEN_ELECTION_TIMER;

    // number of received votes
    self.vote_count = 0;

    // join multicast - create socket to listen on raft multicast messages
    self.listener_sock_fd = init_multicast_listener(&self.l_addr);

    // create socket to send raft multicast messages
    self.sender_sock_fd = init_multicast_sender(&self.s_addr);

    self.log_len = 0;
    self.log_num_of_committed = 0;
}


void join_multicast() {

}


state_t log_update_handler(node_mode_t * node_mode) {

}


////////// FROM RACHEL ///////////

void send_multicast_vote_request()
{
    // send vote request message in format :
    // <type = 'R'> <term>
    char msg[RAFT_MSG_SIZE];
    msg[0] = 'R';
    sprintf(msg+1, "%d", self.term);

    send_msg(self.sender_sock_fd, msg, &self.s_addr);
}

void election_timeout_hndlr(void * p)
{
    // move or stay on candidate state with higher term
    self.node_state = CANDIDATE;
    self.term++;

    // vote for myself
    self.vote_count = 1;

    //restart timer

    //send vote requests
    send_multicast_vote_request();
}

void send_vote(unsigned int term)
{
    // send vote message in format :
    // <type = 'V'> <term>
    char msg[RAFT_MSG_SIZE];
    msg[0] = 'V';
    sprintf(msg+1, "%d", term);

    //assume addr was updated with src ip when got the request
    send_msg(self.listener_sock_fd, msg, &self.l_addr);
}

void vote_req_hndlr(void * p)
{
    unsigned int * term = (int *) p;
    if(*term > self.term)
    {

        self.node_state = FOLLOWER;
        self.term = *term;

        //vote for higher term
        send_vote(*term);

        // restart timer
    }
    // else,
    // one of the following cases:
    // 1. follower has voted already for current term
    // 2. candidate stay with his state and current term
}

/* leader become a follower in the new higher term*/
void node_stepdown(int term)
{
    self.node_state = FOLLOWER;
    self.term = term;
    self.election_timer_interval = GEN_ELECTION_TIMER;

    // number of received votes
    self.vote_count = 0;

    //start timer
}

void l_vote_req_hndlr(void * p)
{
    unsigned int * term = (int *) p;
    if(*term > self.term)
    {
        // leader has to step down
        node_stepdown(*term);

        //vote for higher term
        send_vote(*term);
    }
}

void rollback_uncommitted()
{

    int i = self.log_len -1;
    while(i>=0 && !self.log[i].has_commited && self.log[i].term < self.term)
    {
        self.log_len--;
        i--;
    }
}



int append_entry_msg_hndlr(appendentries_msg_t * ae_msg)
{

    int has_new_leader = 0;
    if(ae_msg->msg_term < self.term)
    {
        // ignore messages from old leader
        return -1;
    }

    if(ae_msg->msg_term > self.term) {

        // new leader has elected
        has_new_leader = 1;
        self.term = ae_msg->msg_term;

        // rollback uncommitted log entries
        rollback_uncommitted();
    }

    // send ack message in format :
    // <type = 'A'> <term>
    char msg[RAFT_MSG_SIZE];
    msg[0] = 'A';
    sprintf(msg+1, "%d", self.term);

    //send ACK
    self.l_addr.sin_addr.s_addr = inet_addr(ae_msg->msg_src_ip);
    send_msg(self.listener_sock_fd, msg, &self.l_addr);

    if(ae_msg->msg_type != HEART_BEAT)
    {
        // add to log
        log_entry_t new_entry;
        new_entry.term = self.term;
        new_entry.type = ae_msg->msg_type;
        strcpy(new_entry.key, ae_msg->key);
        strcpy(new_entry.value, ae_msg->value);
        new_entry.has_commited = 0;

        self.log[self.log_len] = new_entry;
        self.log_len++;
    }

    // restart timer
}

void f_append_entry_msg_hndlr(void * p)
{
    appendentries_msg_t * ae_msg = (appendentries_msg_t *) p;

    append_entry_msg_hndlr(ae_msg);
}

void c_append_entry_msg_hndlr(void * p)
{
    appendentries_msg_t * ae_msg = (appendentries_msg_t *) p;

    int has_new_leader = append_entry_msg_hndlr(ae_msg);
    if(has_new_leader)
    {
        self.node_state = FOLLOWER;
    }
}

void l_append_entry_msg_hndlr(void * p)
{
    appendentries_msg_t * ae_msg = (appendentries_msg_t *) p;

    int has_new_leader = append_entry_msg_hndlr(ae_msg);
    if(has_new_leader)
    {
        node_stepdown(ae_msg->msg_term);
    }

}

void log_update_hndlr(void * p) {

}

//////// END FROM RACHEL ///////////