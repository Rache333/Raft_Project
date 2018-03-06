
#include "log_replication.h"

static const msg_handler_t msg_handlers[MSG_TYPES_N] = {

        [HEARTBEAT] = heartbeat_msg_hndlr,
        [ADD] = add_msg_hndlr,
        [EDIT] = edit_msg_hndlr,
        [DELETE] = delete_msg_hndlr,
        [COMMIT] = commit_msg_hndlr
};

int parse_msg(char * msg, msg_t * mt)
{
    // msg in format : "type,term,key,value,ip"
    msg_type_t type;

    char * buf = strdup(msg);
    char * token = strsep(&buf,","); // token = type
    type = atoi(token);


    token = strsep(&buf,","); // token = term
    mt->term = atoi(token);

    token = strsep(&buf,","); // token = key
    strcpy(mt->key, token);

    token = strsep(&buf,","); // token = value
    strcpy(mt->value, token);

    strcpy(mt->src_ip, buf);

    return type;
}


/* delete uncommitted entries from log */
void rollback_uncommitted()
{

    int i = self.log_len -1;
    while(i>=0 && !self.log[i].has_commited && self.log[i].term < self.term)
    {
        self.log_len--;
        i--;
    }
}

/* send ACK message to reply the leader */
void send_ack_msg(int type, char * ip, int index)
{
    // msg in format : "event = APPENDENTRIES_MSG_ACK, type, term, index"
    char ack_msg[50];
    sprintf(ack_msg, "%d,%d,%d,%d", APPENDENTRIES_MSG_ACK, type, self.term, index);

    //send ACK
    self.l_addr.sin_addr.s_addr = inet_addr(ip);
    send_msg(self.listener_sock_fd, ack_msg, &self.l_addr);
}

void add_new_entry(msg_type_t type, char * key, char * value)
{
    log_entry_t new_entry;
    new_entry.term = self.term;
    new_entry.type = type;
    strcpy(new_entry.key, key);
    strcpy(new_entry.value, value);
    new_entry.has_commited = 0;

    self.log[self.log_len] = new_entry;
    self.log_len++;
}

void follower_msg_hndlr(char * msg)
{

    msg_t mt;
    msg_type_t type = parse_msg(msg, &mt);
    msg_handlers[type](mt);

    // restart timer
    ualarm(self.election_timer_interval, 0);

}
void candidate_msg_hndlr(char * msg)
{
    msg_t mt;
    msg_type_t type = parse_msg(msg, &mt);
    int has_new_leader = msg_handlers[type](mt);
    if(has_new_leader)
    {
        self.node_state = FOLLOWER;
    }
    // restart timer
    ualarm(self.election_timer_interval, 0);
}

void leader_msg_hndlr(char * msg)
{
    msg_t mt;
    msg_type_t type = parse_msg(msg, &mt);
    int has_new_leader = msg_handlers[type](mt);
    if(has_new_leader)
    {
        node_stepdown();
    }

}
int heartbeat_msg_hndlr(msg_t mt)
{
    int has_new_term = 0;
    if(mt.term > self.term) {

        // new leader has elected
        self.term = mt.term;

        // rollback uncommitted log entries
        rollback_uncommitted();

        has_new_term = 1;
    }

    // send ack message
    send_ack_msg(HEARTBEAT, mt.src_ip, -1);

    return has_new_term;
}

int add_msg_hndlr(msg_t mt)
{
    int has_new_term = 0;
    if(mt.term > self.term) {

        // new leader has elected
        self.term = mt.term;

        // rollback uncommitted log entries
        rollback_uncommitted();

        has_new_term = 1;
    }

    // send ack message
    send_ack_msg(ADD, mt.src_ip, self.log_len);

    //add to log
    add_new_entry(ADD, mt.key, mt.value);

    return has_new_term;
}
int edit_msg_hndlr(msg_t mt)
{
    int has_new_term = 0;
    if(mt.term > self.term) {

        // new leader has elected
        self.term = mt.term;

        // rollback uncommitted log entries
        rollback_uncommitted();

        has_new_term = 1;
    }

    // send ack message
    send_ack_msg(EDIT, mt.src_ip, self.log_len);

    //add to log
    add_new_entry(EDIT, mt.key, mt.value);

    return has_new_term;
}
int delete_msg_hndlr(msg_t mt)
{
    int has_new_term = 0;
    if(mt.term > self.term) {

        // new leader has elected
        self.term = mt.term;

        // rollback uncommitted log entries
        rollback_uncommitted();

        has_new_term = 1;
    }

    // send ack message
    send_ack_msg(DELETE, mt.src_ip, self.log_len);

    //add to log
    add_new_entry(DELETE, mt.key, mt.value);

    return has_new_term;
}

//TODO
void commit2redis(char * key, char * value)
{

}

int commit_msg_hndlr(msg_t mt)
{
    int has_new_term = 0;
    if(mt.term > self.term) {

        // new leader has elected
        self.term = mt.term;

        // rollback uncommitted log entries
        rollback_uncommitted();

        has_new_term = 1;
    }

    // send ack message
    send_ack_msg(COMMIT, mt.src_ip, self.log_len);

    /* commit to redis */
    commit2redis(mt.key, mt.value);

    return has_new_term;
}


/* leader got ack msg */
void msg_ack_hndlr(char * cmd)
{
    // msg in format : type, term, index"
    char * buf = strdup(cmd);
    char * token = strsep(&buf, ","); // token = type
    msg_type_t type = (msg_type_t) atoi(token);
    token = strsep(&buf, ","); // token = term
    unsigned int term = (unsigned int) atoi(token);

    if (term < self.term) {
        /* ignore acks from previous terms */
        return;
    }

    int index = atoi(buf);

    // update num of followers acked
    self.log_ack_count[index]++;

    if(self.log_ack_count[index] >= MAJORITY)
    {
        /* send commit request for the entry command */
        char * msg;
        //Msg format: event,type,term,key,value
        sprintf(msg, "%d,%d,%d,%s,%s", (int)APPENDENTRIES_MSG, (int)COMMIT, term,
                self.log[index].key, self.log[index].value);

        send_msg(self.sender_sock_fd, msg, & self.s_addr);

        /* commit to redis */
        commit2redis(self.log[index].key, self.log[index].value);
    }
}