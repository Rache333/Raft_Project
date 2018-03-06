
#include "log_update_commands.h"


int delete_cmd(char * key) {

    if (self.node_state != LEADER) {
        return -1;
    }

    mqd_t mq;
    char * str;
    sprintf(str, "%d,%d,%s%c",(int)LOG_UPDATE, (int)UPDATE_DELETE, key, '\0');
    /* open the mail queue */
    mq = mq_open(EVENT_QUEUE, O_WRONLY);
    mq_send(mq, str, sizeof(str), 0);

    return 0;

}

int edit_cmd(char* key, char* value) {

    if (self.node_state != LEADER) {
        return -1;
    }

    mqd_t mq;
    char * str;
    sprintf(str, "%d,%d,%s,%s%c",(int)LOG_UPDATE, (int)UPDATE_EDIT, key, value, '\0');
    /* open the mail queue */
    mq = mq_open(EVENT_QUEUE, O_WRONLY);
    mq_send(mq, str, sizeof(str), 0);

    return 0;
}

int show_node_status() {
    return (int)(self.node_state);
}

void show_system_status() {

    struct ifaddrs *addrs;
    struct ifaddrs *tmp;

    getifaddrs(&addrs);
    tmp = addrs;

    while (tmp)
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
        {
            if(!StartsWith(tmp->ifa_name,"docker") && !StartsWith(tmp->ifa_name,"lo")) {
                struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
                printf("This node ip is: %s\n", inet_ntoa(pAddr->sin_addr));
            }
        }

        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
}

void show_log() {

    char *msg_type[5] = {"ADD", "EDIT","DELETE", "HEARTBEAT", "COMMIT" };
    char *has_commited[2] = {"FALSE", "TRUE"};

    if(self.log_len == 0) {
        puts("The log is empty");
    }
    else {
        puts("This node log is: ");
        puts("-----------------");
        puts("Index     Term     Msg type     key     value     Commited");
        for(int i = 0 ; i < self.log_len ; i++) {
            printf("%d     %d     %s     %s     %s     %s\n",i,self.log[i].term, msg_type[self.log[i].type], self.log[i].key, self.log[i].value, has_commited[self.log[i].has_commited]);
        }
    }

}

int StartsWith(const char *a, const char *b)
{
    if(strncmp(a, b, strlen(b)) == 0){
        return 1;
    }
    else {
        return 0;
    }

}