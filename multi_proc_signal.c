#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/wait.h>

int conc_amnt = 0;

void pa_handle_sigchld(int signum){
    //if (signum == SIGCHLD){
        pid_t pid;
        int status;
        printf("[pid](%d): SIGCHLD is handled in pa_handle_sigchld()\n", getpid());
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0){
            if (!WIFSIGNALED(status)){
                printf("[pid](%d): [cpid](%d) is waited in pa_handle_sigchld()\n", getpid(), pid);
                conc_amnt --;
            }
        }
    //}
}

void pa_handle_sigint(int signum){
    if (signum == SIGINT){
        printf("[pid](%d): SIGINT is handled in pa_handle_sigint()\n", getpid());
        printf("[pid](%d): [func](exit) with [conc_amnt](%d)\n", getpid(), conc_amnt);
        exit(1);
    }
}

int main (int argc, char* argv[]){
    conc_amnt = atoi(argv[2]);
    printf("[pid](%d): start\n", getpid());
    pid_t children[conc_amnt-1];
    struct sigaction act1, act2;
    memset(&act1, 0, sizeof(struct sigaction));
    memset(&act2, 0, sizeof(struct sigaction));
    sigemptyset(&act1.sa_mask);
    sigemptyset(&act2.sa_mask);
    act1.sa_handler = pa_handle_sigint;
    act2.sa_handler = pa_handle_sigchld;
    act1.sa_flags = 0;
    act2.sa_flags = 0;
    sigaction(SIGINT, &act1, NULL);
    sigaction(SIGCHLD, &act2, NULL);
    //derive child process
    int i;
    for (i = 0; i < conc_amnt-1; i ++){
        if ((children[i] = fork()) == 0) break;
    }
    if (children[i] == 0){
        printf("[pid](%d): start\n", getpid());
        pause();
    } else {
        sleep(1);
        for (int j = 0; j < conc_amnt-1; j ++){
            kill(children[j], 2);
        }
        while (conc_amnt > 1){
            printf("[pid](%d): [func](pause) with [conc_amnt](%d)\n", getpid(), conc_amnt);
            pause();
        }
        pause();
    }
    return 0;
}