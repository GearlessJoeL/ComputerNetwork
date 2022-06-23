#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_CMD_STR 100

// 业务逻辑处理函数
void echo_rqt(int sockfd) {
    char buf[MAX_CMD_STR + 1];
    while (fgets(buf, MAX_CMD_STR, stdin)) {
        if(strncmp(buf, "exit", 4) == 0){
            return;
        }

        int len_h = strnlen(buf, 100);
        int len_n = htonl(len_h);
        char* temp = NULL;
        if ((temp = strstr(buf, "\n"))) *temp = '\0';

        write(sockfd, &len_n, sizeof(len_n));
        write(sockfd, buf, len_h);

        read(sockfd, &len_n, sizeof(len_n));
        int res = 0;
        while(res != len_h){
            read(sockfd, buf + res, 1);
            res ++;
        }
        printf("[echo_rep] %s\n", buf);  
    }
    //return 0;
}

int main(int argc, char *argv[]){

    int	connfd;
    struct sockaddr_in srv_addr;
    if ((connfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Create socket failed.\n");
        exit(1);
    }
    if(argc != 3){
        printf("Usage:%s <IP> <PORT>\n", argv[0]);
        return 0;
    }
    bzero(&srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, argv[1], &srv_addr.sin_addr) == 0){
        perror("Server IP Address Error:\n");
        exit(1);
    }
    srv_addr.sin_port = htons(atoi(argv[2]));
    int res = connect(connfd, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr));

    if(res == 0){
        printf("[cli] server[%s:%s] is connected!\n", argv[1], argv[2]);
        echo_rqt(connfd);
    }
    else if(res == -1){
        printf("[cli] connect error! errno is %d\n", errno);
    }
    close(connfd);

    printf("[cli] connfd is closed!\n");
    printf("[cli] client is exiting!\n");
    return 0;
}

