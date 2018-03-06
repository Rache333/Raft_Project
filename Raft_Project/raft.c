
#include "raft.h"


static pthread_t run_t;
static pthread_t rx;

callback_types_t callback_type;

typedef void (*event_handler_t)(char *);

static event_handler_t event_handlers[STATE_N][EVENT_N] = {
        [FOLLOWER][ELECTION_TIMEOUT] = election_timeout_hndlr,
        [CANDIDATE][ELECTION_TIMEOUT] = election_timeout_hndlr,
        [LEADER][ELECTION_TIMEOUT] = send_heartbeat,

        [FOLLOWER][VOTE_REQUEST] = vote_req_hndlr,
        [CANDIDATE][VOTE_REQUEST] = vote_req_hndlr,
        [LEADER][VOTE_REQUEST] = l_vote_req_hndlr,

        [CANDIDATE][VOTE] = vote_hndlr,

        [FOLLOWER][APPENDENTRIES_MSG] = follower_msg_hndlr,
        [CANDIDATE][APPENDENTRIES_MSG] = candidate_msg_hndlr,
        [LEADER][APPENDENTRIES_MSG] = leader_msg_hndlr,

        [LEADER][APPENDENTRIES_MSG_ACK] = msg_ack_hndlr,

        [LEADER][LOG_UPDATE] = log_update_hndlr
};

//TODO
static event_handler_t log_update_handlers[UPDATE_TYPES_N] = {
        [UPDATE_ADD] = update_add_hndlr,
        [UPDATE_EDIT] = update_edit_hndlr,
        [UPDATE_DELETE] = update_delete_hndlr
};


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
    mq_unlink(EVENT_QUEUE);
    mq = mq_open(EVENT_QUEUE, O_CREAT | O_RDONLY, 0644, &attr);
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
    printf("Got sigalrm.\n");
    event_handlers[self.node_state][ELECTION_TIMEOUT](NULL);
}

void node_init() {

    signal(SIGALRM, sigalarm_hndr);

    self.node_state = FOLLOWER;
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

    ualarm(self.election_timer_interval, 0);

}


void * listen_to_msgs(void * p) {

    char msg[MSG_SIZE];
    char cmd[MSG_SIZE];
    char s_addr[32]; // for saving src ip
    mqd_t mq;
//    struct sockaddr_in sockaddrIn;
//    sockaddrIn.sin_family = AF_INET;
//    sockaddrIn.sin_port = htons(PORT);
    mq = mq_open(EVENT_QUEUE, O_WRONLY);

    while(1) {


        recv_msg(self.listener_sock_fd,(struct sockaddr_in *) &self.l_addr ,msg, s_addr , MSG_SIZE);
        puts(s_addr);

        char* buf = strdup(msg);
        char* id = strsep(&buf, ",");
        if(NODE_ID == atoi(id))
        {
            printf("msg droped = %s with my id = %s\n", buf, id);
            continue;
        }
//        /* drop msgs you send to yourself in multi-cast */
//        if(!strcmp(s_addr, MY_IP)) {
//            printf("msg droped = %s from ip = %s\n", msg, s_addr);
//            continue; }

        //sockaddrIn.sin_addr.s_addr = inet_addr(s_addr);
        //send_msg(self.sender_sock_fd, "Ack from reciever", &sockaddrIn);

        sprintf(cmd, "%s,%s", buf, s_addr);
        printf("%s, ip = %s, size is %d\n", cmd, s_addr, (int) strlen(s_addr));

        //fflush(stdout);

        mq_send(mq, cmd, sizeof(cmd), 0);
        printf("done writing %s to the queue ...\n", cmd);
    }

}

void log_update_hndlr(char * cmd) {

    char * cmd_str = (char *) cmd;
    char new_cmd[MSG_SIZE];
    sprintf(new_cmd, "%d,%s%c",(int)APPENDENTRIES_MSG, cmd_str, '\0');
    send_msg(self.sender_sock_fd, new_cmd, &self.s_addr);
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


void send_heartbeat(char * cmd)
{
    // send heartbeat message in format :
    // <event> <type> <term>
    char msg[RAFT_MSG_SIZE];
    sprintf(msg, "%d,%d,%d%c", (int)APPENDENTRIES_MSG, (int)HEARTBEAT, self.term, '\0');

    send_msg(self.sender_sock_fd, msg, &self.s_addr);
}


/* leader become a follower in the new higher term*/
void node_stepdown()
{
    self.node_state = FOLLOWER;
    self.election_timer_interval = GEN_ELECTION_TIMER;

    // number of received votes
    self.vote_count = 0;

    // restart timer
    ualarm(self.election_timer_interval, 0);

}