#include "raft.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

static event_handler_t event_handlers[STATE_N][EVENT_N] = {
        [LEADER][LOG_UPDATE] = log_update_handler
};
delete_callback_t delete_callback;
edit_callback_t edit_callback;


void run(delete_callback_t delete, edit_callback_t edit) {
    delete_callback = delete;
    edit_callback = edit;
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


state_t log_update_handler(node_mode_t * node_mode) {

}
