#include "raft.h"
#include <stdio.h>
#include <time.h>

static event_handler_t event_handlers[STATE_N][EVENT_N] = {
        [LEADER][LOG_UPDATE] = handle_log_update
};


void init() {
    time_t t;
    srand((unsigned)time(&t));
    node_init(& self);
    join_multicast();

}


void node_init(node_mode_t * node_mode) {
    node_mode->node_state = FOLLOWER;
    node_mode->term = 0;
    node_mode->election_timer_interval = GEN_ELECTION_TIMER;
    node_mode->vote_count = 0;
}


void join_multicast() {

}
state_t handle_log_update(node_mode_t * node_mode) {

}