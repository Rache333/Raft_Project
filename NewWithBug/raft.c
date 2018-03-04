#include "raft.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "multicast_handling.h"

static pthread_t run_t;
static pthread_t rx;

//static event_handler_t event_handlers[STATE_N][EVENT_N] = {
//        [LEADER][LOG_UPDATE] = log_update_handler
//};

callback_types_t callback_type;


void run(void * a) {


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

    pthread_create(&rx,NULL, &listen_to_msgs,NULL);

    while(1) {

        ssize_t bytes_read;

        /* receive the message */
        bytes_read = mq_receive(mq, buffer, MSG_SIZE, NULL);
        if(bytes_read == -1) {
            printf("%s", strerror(errno));
        }
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);

        char* buf = strdup(buffer);
        char* str_cmd = strsep(&buf, ",");

        event_handlers[self.node_state][atoi(str_cmd)](buf);

    }
}

void init(delete_callback_t _delete1, edit_callback_t _edit1) {

    callback_type._delete = _delete1;
    callback_type._edit = _edit1;
    pthread_create(&run_t,NULL, &run,NULL);

}


void node_init() {

    signal(SIGALRM, election_timeout_hndlr);

    self.node_state = LEADER;
    //printf("node status: %d", (unsigned int)&self);
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

    //ualarm(self.election_timer_interval, 0);

}


void listen_to_msgs() {

    char msg[MSG_SIZE];
    char s_addr[32];
    mqd_t mq;
    struct sockaddr_in sockaddrIn;
    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_port = htons(PORT);

    while(1) {
        if(self.node_state == LEADER) {

            printf("Receiving...\n");
            //fflush(stdout);

            recv_msg(self.listener_sock_fd,(struct sockaddr_in *) &self.l_addr ,msg, s_addr , MSG_SIZE);
            puts(s_addr);
            sockaddrIn.sin_addr.s_addr = inet_addr(s_addr);
            send_msg(self.sender_sock_fd, "Ack from reciever", &sockaddrIn);
            printf("%s\n", msg);
            //fflush(stdout);
            mq = mq_open("/Event queue", O_WRONLY);
            mq_send(mq, msg, sizeof(msg), 0);
            printf("done writing %s to the queue ...", msg);
        }
    }

}


void log_update_hndlr(void* cmd) {

    char * cmd_str = (char *) cmd;
    send_msg(self.sender_sock_fd, "hello", &self.s_addr);
    puts("the msg was sent!");
}

void update_delete_hndlr(void * cmd) {
    char* cmd_str = (char* )cmd;

}

void update_add_hndlr(void * cmd) {
    char* cmd_str = (char* )cmd;

}

void update_edit_hndlr(void * cmd) {
    char* cmd_str = (char* )cmd;

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

    //send vote requests
    send_multicast_vote_request();

    ualarm(self.election_timer_interval, 0);
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

        ualarm(self.election_timer_interval, 0);
    }

    /* else,
     * one of the following cases:
     * 1. follower has voted already for current term
     * 2. candidate stay with his state and current term
     */
}

/* leader become a follower in the new higher term*/
void node_stepdown(int term)
{
    self.node_state = FOLLOWER;
    self.term = term;
    self.election_timer_interval = GEN_ELECTION_TIMER;

    // number of received votes
    self.vote_count = 0;

    ualarm(self.election_timer_interval, 0);

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

//////// END FROM RACHEL ///////////