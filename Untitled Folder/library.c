#include <stdio.h>

int edit_cmd(char* a, char* b, void (*f)(void)) {
    printf("From c: you changed %s to %s\n", a,b);
    f();
    return 0;
}
