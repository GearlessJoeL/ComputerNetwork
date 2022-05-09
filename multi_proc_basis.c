#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#define buffersize 5

int main (int argc, char* argv[]){
    printf("[pid](%d) [ppid](%d): start\n", getpid(), getppid());
    int n = atoi(argv[2]);
    pid_t children[n-1];
    int fd1_td1 = open("td1.txt", O_RDONLY);
    //parent
    printf("[pid](%d) [ppid](%d): [fd1_td1](%d) is got\n", getpid(), getppid(), fd1_td1);

    for (int i = 0; i < n-1; i ++){
        if ((children[i] = fork()) == 0){
            //child
            printf("[pid](%d) [ppid](%d): start\n", getpid(), getppid());
            //use fd1_td1 to read file1
            char buffer1[buffersize];
            int flag1 = read(fd1_td1, buffer1, buffersize*sizeof(char));
            printf("[pid](%d) [ppid](%d): [read](%s) by [fd1_td1](%d)\n", getpid(), getppid(), buffer1, fd1_td1);
            //create fd2_td1 to read file1
            int fd2_td1 = open("td1.txt", O_RDONLY);
            printf("[pid](%d) [ppid](%d): [fd2_td1](%d) is got\n", getpid(), getppid(), fd2_td1);
            char buffer2[buffersize];
            int flag2 = read(fd2_td1, buffer2, buffersize*sizeof(char));
            printf("[pid](%d) [ppid](%d): [read](%s) by [fd2_td1](%d)\n", getpid(), getppid(), buffer2, fd2_td1);
            close(fd2_td1);
            close(fd1_td1);
            printf("[pid](%d) [ppid](%d): exit\n", getpid(), getppid());
            exit(0);
        }
    }

    //use fd1_td1 to read file1
    char buffer1[buffersize];
    int flag1 = read(fd1_td1, buffer1, buffersize*sizeof(char));
    printf("[pid](%d) [ppid](%d): [read](%s) by [fd1_td1](%d)\n", getpid(), getppid(), buffer1, fd1_td1);
    close(fd1_td1);
    //use fd1_td2 to read file2
    char buffer2[buffersize];
    int fd1_td2 = open("td2.txt", O_RDONLY);
    printf("[pid](%d) [ppid](%d): [fd1_td2](%d) is got\n", getpid(), getppid(), fd1_td2);
    int flag2 = read(fd1_td2, buffer2, buffersize*sizeof(char));
    printf("[pid](%d) [ppid](%d): [read](%s) by [fd1_td2](%d)\n", getpid(), getppid(), buffer2, fd1_td2);
    close(fd1_td2);
    sleep(2);
    printf("[pid](%d) [ppid](%d): exit\n", getpid(), getppid());
    exit(0);

    return 0;
}