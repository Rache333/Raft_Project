
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

typedef enum states {
    START = 0,
    LOOP,
    END,
} state_t;

typedef enum events {
    START_LOOPING = 0,
    PRINT_HELLO,
    STOP_LOOPING,
} event_t;

typedef state_t (*event_handler)(state_t, event_t);

state_t start_looping(state_t state, event_t event) {
    assert(state == START && event == START_LOOPING);
    return LOOP;
}

state_t print_hello(state_t state, event_t event) {
    assert(state == LOOP && event == PRINT_HELLO);
    printf("Hello World!\n");
    return LOOP;
}

state_t stop_looping(state_t state, event_t event) {
    assert(state == LOOP && event == STOP_LOOPING);
    return END;
}

event_handler transitions[END+1][STOP_LOOPING+1] = {
        [START][START_LOOPING] = start_looping,
        [LOOP][PRINT_HELLO] = print_hello,
        [LOOP][STOP_LOOPING] = stop_looping,
};

state_t step_state(state_t state, event_t event) {
    event_handler handler = transitions[state][event];
    if (!handler)
        exit(1);
    return handler(state, event);
}

int main(void) {
    state_t state = START;
    printf("%d\n", state);
    state = step_state(state, START_LOOPING);
    sleep(3);
    printf("%d\n", state);
    state = step_state(state, PRINT_HELLO);
    sleep(3);
    printf("%d\n", state);
    state = step_state(state, PRINT_HELLO);
    sleep(3);
    printf("%d\n", state);
    state = step_state(state, STOP_LOOPING);
    sleep(3);
    printf("%d\n", state);
    return 0;
}