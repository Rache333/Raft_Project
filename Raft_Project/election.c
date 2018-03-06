
#include "election.h"

/* candidate send requests to vote for him */
void send_multicast_vote_request()
{
    // send vote request message in format :
    // <event> <term>
    char msg[RAFT_MSG_SIZE];
    sprintf(msg, "%d,%d%c", (int)VOTE_REQUEST, self.term, '\0');

    send_msg(self.sender_sock_fd, msg, &self.s_addr);
}

void election_timeout_hndlr(char * cmd)
{
    printf("Running election_timeout_hndlr.\n");
    // move or stay on candidate state with higher term
    self.node_state = CANDIDATE;
    self.term++;

    // vote for myself
    self.vote_count = 1;

    //send vote requests
    send_multicast_vote_request();

    ualarm(self.election_timer_interval, 0);
}

/* vote for the candidate */
void send_vote(unsigned int term, char * ip)
{
    // send vote message in format :
    // <event> <term>
    char msg[RAFT_MSG_SIZE];
    sprintf(msg, "%d,%d%c", (int)VOTE, self.term, '\0');

    // set destination ip of the candidate
    self.l_addr.sin_addr.s_addr = inet_addr(ip);
    send_msg(self.listener_sock_fd, msg, &self.l_addr);
}

/* when follower or candidate got request to vote */
void vote_req_hndlr(char * cmd)
{
    // cmd message in format :
    // <term> <ip>
    char * buf = strdup(cmd);
    char * token = strsep(&buf, ",");
    unsigned int term = (unsigned int) atoi(token);

    if(term > self.term)
    {

        self.node_state = FOLLOWER;
        self.term = term;

        //vote for higher term
        send_vote(term, buf);

        ualarm(self.election_timer_interval, 0);
    }

    /* else,
     * one of the following cases:
     * 1. follower has voted already for current term
     * 2. candidate stay with his state and current term
     */
}

void l_vote_req_hndlr(char * cmd)
{
    // cmd message in format :
    // <term> <ip>
    char * buf = strdup(cmd);
    char * token = strsep(&buf, ",");
    unsigned int term = (unsigned int) atoi(token);

    if(term > self.term)
    {
        // leader has to step down
        self.term = term;
        node_stepdown();

        //vote for higher term
        send_vote(term, buf);
    }
}


void vote_hndlr(char * cmd) {

    // cmd message in format :
    // <term> <ip>
    char * buf = strdup(cmd);
    char * token = strsep(&buf, ",");
    unsigned int term = (unsigned int) atoi(token);

    if(term == self.term)
    {
        puts("got a vote! :D");
        self.vote_count++;

        if(self.vote_count >= MAJORITY)
        {
            puts("**** become a leader!!! ****");
            self.node_state = LEADER;
        }
    }
}