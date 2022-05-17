#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#define buffersize 61

int conc_amnt = 0;
u_int tgid;

void *func1(void* args){
    char* str = (char*)args;
    printf("[spid](%lu) [tgid](%u) [ptid](%lu) start\n", syscall(SYS_gettid), tgid, pthread_self());
    printf("[spid](%lu) [read](%s)\n", syscall(SYS_gettid), str);
    printf("[spid](%lu) [tgid](%u) [ptid](%lu) [func](pthread_exit)\n", syscall(SYS_gettid), tgid, pthread_self());
    pthread_exit(NULL);
}

void *func2(void* args){
    pthread_detach(pthread_self());
    char* str = (char*)args;
    printf("[spid](%lu) [tgid](%u) [ptid](%lu) start\n", syscall(SYS_gettid), tgid, pthread_self());
    printf("[spid](%lu) [read](%s)\n", syscall(SYS_gettid), str);
    printf("[spid](%lu) [tgid](%u) [ptid](%lu) [func](pthread_exit)\n", syscall(SYS_gettid), tgid, pthread_self());
    pthread_exit(NULL);
}

int main (int argc, char* argv[]){
    conc_amnt = atoi(argv[2]);
    char buffer[conc_amnt][buffersize];
    tgid = getpid();
    printf("[spid](%u) [tgid](%u) [ptid](%lu) start\n", getpid(), tgid, pthread_self());
    FILE* fp = fopen("td.txt", "r");
    pthread_t thd[conc_amnt-1];
    if (conc_amnt > 1) {
        fgets(buffer[0], buffersize*sizeof(char), fp);
        char* temp = NULL;
        if ((temp = strstr(buffer[0], "\n"))) *temp = '\0';
        printf("[spid](%u) [read](%s)\n", getpid(), buffer[0]);
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&thd[0], &attr, &func1, (void*)buffer[0]);
        pthread_attr_destroy(&attr);
        
    }
    for (int i = 1; i < conc_amnt-1; i ++){
        fgets(buffer[i], buffersize*sizeof(char), fp);
        char* temp = NULL;
        if ((temp = strstr(buffer[i], "\n"))) *temp = '\0';
        printf("[spid](%u) [read](%s)\n", getpid(), buffer[i]);
        pthread_create(&thd[i], NULL, &func2, (void*)buffer[i]);
        //pthread_detach(thd[i]);
    }
    
    /*
    for (int i = 0; i < conc_amnt-1; i ++){
        pthread_join(thd[i], NULL);
    }
    */
    fclose(fp);
    sleep(1);
    printf("[spid](%u) [tgid](%u) [ptid](%lu) [func](exit)\n", getpid(), tgid, pthread_self());
    exit(0);
    return 0;
}