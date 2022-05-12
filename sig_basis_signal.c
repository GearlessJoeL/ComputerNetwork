#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void sighandler(int signum){
    if (signum == SIGINT){
        printf("[pid](%d): [sig](SIGINT)  is captured\n", getpid());
        printf("[pid](%d): exit\n", getpid());
        exit(0);
    } else if (signum == SIGUSR1){
        printf("[pid](%d): [sig](SIGUSR1) is captured\n", getpid());
    } else if (signum == SIGUSR2){
        printf("[pid](%d): [sig](SIGUSR2) is captured\n", getpid());
    }
}

int main(){
    printf("[pid](%d): start\n", getpid());
    signal(SIGINT, sighandler);
    signal(SIGUSR1, sighandler);
    signal(SIGUSR2, sighandler);
    while(1) {
        pause();
    }
    return 0;
}