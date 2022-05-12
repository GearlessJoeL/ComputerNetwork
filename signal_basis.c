#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#define _XOPEN_SOURCE

void sighandler(int signum){
    if (signum == SIGINT){
        printf("[pid](%d): [sig](SIGINT) is captured\n", getpid());
        printf("[pid](%d): exit\n", getpid());
        exit(1);
    } else if (signum == SIGUSR1){
        printf("[pid](%d): [sig](SIGUSR1) is captured\n", getpid());
    } else if (signum == SIGUSR2){
        printf("[pid](%d): [sig](SIGUSR2) is captured\n", getpid());
    }
}

int main(){
    printf("[pid](%d): start\n", getpid());
    struct sigaction act, oldact;
    memset(&act, 0, sizeof(struct sigaction));
    memset(&oldact, 0, sizeof(struct sigaction));
    sigemptyset(&act.sa_mask);
    act.sa_handler = sighandler;
    act.sa_flags = 0;//SA_NODEFER|SA_RESETHAND;//SA_ONESHOT|SA_NOMASK;
    sigaction(SIGINT, &act, &oldact);
    sigaction(SIGUSR1, &act, &oldact);
    sigaction(SIGUSR2, &act, &oldact);
    while(1){
        pause();
    }
    return 0;
}