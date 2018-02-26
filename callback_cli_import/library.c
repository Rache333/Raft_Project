#include <stdio.h>

int edit_cmd(char* a, char* b, void (*f)(char*, char*)) {
    printf("From c: you changed %s to %s\n", a,b);
    f(a,b);
    return 0;
}

int delete_cmd(char* a, void (*f)(char*)) {
    printf("From c: you deleted %s\n", a);
    f(a);
    return 0;
}

