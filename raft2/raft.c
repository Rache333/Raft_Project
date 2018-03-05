
#include "raft.h"


static pthread_t run_t;
static pthread_t rx;




callback_types_t callback_type;


void * run(void * a) {


    time_t t;
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_curmsgs = 0;
    srand((unsigned)time(&t));
    node_init();
    mqd_t mq;
    mq_unlink("/Event queue");
    mq = mq_open("/Event queue", O_CREAT | O_RDONLY, 0644, &attr);
    char buffer[MSG_SIZE];

    pthread_create(&rx,NULL, listen_to_msgs,NULL);

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
    pthread_create(&run_t,NULL, run,NULL);

}

void sigalarm_hndr(int p)
{
    if(self.node_state == LEADER)
    {
       // heartbeat timeout
        //TODO
    } else {
        election_timeout_hndlr(NULL);
    }
}

void node_init() {

    signal(SIGALRM, sigalarm_hndr);

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
    memset(self.log_ack_count, 0, sizeof(self.log_ack_count) / sizeof(self.log_ack_count[0]));

    //ualarm(self.election_timer_interval, 0);

}


void * listen_to_msgs(void * p) {

    char msg[MSG_SIZE];
    char s_addr[32];
    mqd_t mq;
    struct sockaddr_in sockaddrIn;
    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_port = htons(PORT);
    mq = mq_open("/Event queue", O_WRONLY);

    while(1) {


        recv_msg(self.listener_sock_fd,(struct sockaddr_in *) &self.l_addr ,msg, s_addr , MSG_SIZE);
        puts(s_addr);
        sockaddrIn.sin_addr.s_addr = inet_addr(s_addr);
        //send_msg(self.sender_sock_fd, "Ack from reciever", &sockaddrIn);
        printf("%s\n", msg);
        //fflush(stdout);
        mq_send(mq, msg, sizeof(msg), 0);
        printf("done writing %s to the queue ...\n", msg);

    }

}


void log_update_hndlr(char * cmd) {

    char * cmd_str = (char *) cmd;
    char new_cmd[MSG_SIZE];
    sprintf(new_cmd, "%d,%s%c",(int)APPENDENTRIES_MSG, cmd_str, '\0');
    //send_msg(self.sender_sock_fd, new_cmd, &self.s_addr);
    puts("the msg was sent!\n");
    printf("%s was sent", new_cmd);
}

void update_delete_hndlr(char * cmd) {
    char* cmd_str = (char* )cmd;

}

void update_add_hndlr(char * cmd) {
    char* cmd_str = (char* )cmd;

}

void update_edit_hndlr(char * cmd) {
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

void election_timeout_hndlr(char * p)
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

void vote_req_hndlr(char * p)
{
    unsigned int * term = (unsigned int *) p;
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
void node_stepdown()
{
    self.node_state = FOLLOWER;
    self.election_timer_interval = GEN_ELECTION_TIMER;

    // number of received votes
    self.vote_count = 0;

    ualarm(self.election_timer_interval, 0);

}

void l_vote_req_hndlr(char * p)
{
    unsigned int * term = (unsigned int *) p;
    if(*term > self.term)
    {
        // leader has to step down
        self.term = *term;
        node_stepdown();

        //vote for higher term
        send_vote(*term);
    }
}


void vote_hndlr(char * cmd) {
    
}