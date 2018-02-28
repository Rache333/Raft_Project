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

    printf("hello from c proc num: %d\n", getpid());
    fflush(stdout);
    printf("%d, %d\n", (int)callback_type._delete, (int)callback_type._edit);

    time_t t;
    srand((unsigned)time(&t));
    node_init(& self);
    join_multicast();


}

void init(delete_callback_t _delete1, edit_callback_t _edit1) {

    callback_type._delete = _delete1;
    callback_type._edit = _edit1;
    printf("%d, %d\n", (int)callback_type._delete, (int)callback_type._edit);
    pthread_create(&t1,NULL, &run,NULL);
    puts("hi");
    sleep(3);

}


void node_init(node_mode_t * node_mode) {
    node_mode->node_state = FOLLOWER;
    node_mode->term = 0;
    node_mode->election_timer_interval = GEN_ELECTION_TIMER;
    node_mode->vote_count = 0;
}


void join_multicast() {
    puts("hi from the other thread");
}


state_t log_update_handler(node_mode_t * node_mode) {

}
