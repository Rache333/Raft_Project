#include "log_replication.h"


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
    strcpy(self.l_addr.sin_addr.s_addr,ip);
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
}

/* leader become a follower in the new higher term*/
void node_stepdown()
{
    self.node_state = FOLLOWER;
    self.election_timer_interval = GEN_ELECTION_TIMER;

    // number of received votes
    self.vote_count = 0;

    //start timer
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
    send_ack_msg(HEART_BEAT, mt.src_ip, -1);

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

    // update redis

    return has_new_term;
}
