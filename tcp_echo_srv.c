#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

//int sig_to_exit = 0;
int sig_type = 0;

void sig_int(int signum) {
    printf("[srv] SIGINT is coming!\n");
    sig_type = signum;
    //sig_to_exit = 1;
}

void sig_pipe(int signum) {
    sig_type = signum;
    printf("[srv] SIGPIPE is coming!\n");
}

// 业务逻辑处理函数；
void echo_rep(int sockfd) {
    
    int len, res;
    int len_h;
    int len_n;

    char buf[101];
    while(1){
        res = read(sockfd, &len_n, sizeof(len_n));
        len_h = ntohl(len_n);
        
        if(res == 0) return ;
        if(res < 0){
            printf("[srv] read len return %d and errno is %d\n", res, errno);
            if(errno == EINTR){
                if(sig_type == SIGINT) return;
                continue;
            }
            return;
        }
        
        int temp = 0;
        while(1){
            res = read(sockfd, buf + temp, 1);
            temp++;
            if(res == 0) return ;
            if(res < 0){
                printf("[srv] read data return %d and errno is %d\n", res, errno);
                if(errno == EINTR){
                    if(sig_type == SIGINT){
                        free(buf);
                        return;
                    }
                    continue;
                }
                free(buf);
                return;
            }
            
            if(temp == len_h) break;
        }
        printf("[echo_rqt] %s\n", buf);
        write(sockfd, &len_n, sizeof(len_n));
        write(sockfd, buf, len_h);
    }
}

int main(int argc, char *argv[]){
    int sin_size = sizeof(struct sockaddr_in);
    
    struct sigaction sigact_pipe, old_sigact_pipe, sigact_int, old_sigact_int;
    sigact_pipe.sa_handler = sig_pipe;
    sigact_int.sa_handler = sig_int;
    sigemptyset(&sigact_pipe.sa_mask);
    sigemptyset(&sigact_int.sa_mask);
    sigact_pipe.sa_flags = 0;
    sigact_int.sa_flags = 0;
    sigact_pipe.sa_flags |= SA_RESTART;
    sigaction(SIGPIPE, &sigact_pipe, &old_sigact_pipe);
    sigaction(SIGINT, &sigact_int, &old_sigact_int);
    printf("[srv] server is initializing!\n");

    struct sockaddr_in srv_addr, cli_addr;
    socklen_t cli_addr_len;
    int listenfd, connfd;	
    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &srv_addr.sin_addr);
    srv_addr.sin_port = htons(atoi(argv[2]));
    printf("[srv] server[%s:%s] is initializing!\n", argv[1], argv[2]);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int res = bind(listenfd, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr));
    res = listen(listenfd, 5);
    connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &sin_size);
    printf("connfd = %d\n", connfd);
    char buf2[20] = { 0 };
    inet_ntop(AF_INET, &cli_addr.sin_addr, buf2, sizeof(buf2));
    printf("[srv] client[%s:%d] is accepted!\n", buf2, htons(cli_addr.sin_port));
    echo_rep(connfd);
    close(connfd);
    printf("[srv] connfd is closed!\n");
    connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &sin_size);
    if(connfd < 0){
        close(listenfd);
        printf("[srv] listenfd is closed!\n");
        printf("[srv] server is exiting\n");
        return 0;
    }
    close(listenfd);
    printf("[srv] listenfd is closed!\n");
    printf("[srv] server is exiting\n");
    return 0;
}

