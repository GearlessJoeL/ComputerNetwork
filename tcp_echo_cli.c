#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CMD_STR 100
#define bprintf(fp, format, ...) \
	if(fp == NULL){printf(format, ##__VA_ARGS__);} 	\
	else{printf(format, ##__VA_ARGS__);	\
			fprintf(fp, format, ##__VA_ARGS__);fflush(fp);}

int sig_type = 0;
FILE * fp_res = NULL;

void sig_pipe(int signum) {
	sig_type = signum;
	pid_t pid = getpid();
	bprintf(fp_res, "[cli](%d) SIGPIPE is coming!\n", pid);	
}
void sig_chld(int signum) {
    sig_type = signum;
	pid_t pid = getpid(), pid_chld = 0;
    int stat; 
	bprintf(fp_res, "[cli](%d) SIGCHLD is coming!\n", pid);
    while ((pid_chld = waitpid(-1, &stat, WNOHANG)) > 0){
		bprintf(fp_res, "[cli](%d) child process(%d) terminated.\n", pid, pid_chld);
	}
}

void echo_rqt(int sockfd, int pin){
	pid_t pid = getpid();
	int len_h = 0, len_n = 0;
	int pin_h = pin, pin_n = htonl(pin);
	char fn_td[10] = {0};
	char buf[MAX_CMD_STR+1+8] = {0};
	sprintf(fn_td, "td%d.txt", pin);
	FILE * fp_td = fopen(fn_td, "r");
	if(!fp_td){
		bprintf(fp_res, "[cli](%d) Test data read error!\n", pin_h);
		return;
	}
    while (fgets(buf+8, MAX_CMD_STR, fp_td)) {
		pin_h = pin;
		pin_n = htonl(pin);
		if(strncmp(buf+8, "exit", 4) == 0) break;
		memcpy(buf, &pin_n, 4);
		len_h = strnlen(buf+8, MAX_CMD_STR);
		len_n = htonl(len_h);
		memcpy(buf+4, &len_n, 4);
		char* temp = NULL;
        if ((temp = strstr(buf, "\n"))) *temp = '\0';
        write(sockfd, buf, len_h+8);
		memset(buf, 0, sizeof(buf));
        read(sockfd, &pin_n, 4);
		read(sockfd, &len_n, 4);
		len_h = ntohl(len_n);
        read(sockfd, buf, len_h);
		bprintf(fp_res,"[echo_rep](%d) %s\n", pid, buf);
    }
}

int main(int argc, char* argv[]){
	if(argc != 4){
		printf("Usage:%s <IP> <PORT> <CONCURRENT AMOUNT>\n", argv[0]);
		return 0;
	}
	struct sigaction sigact_pipe, old_sigact_pipe;
	struct sigaction sigact_chld, old_sigact_chld;
	sigact_pipe.sa_handler = &sig_pipe;
    sigact_chld.sa_handler = &sig_chld;
	sigact_pipe.sa_flags = 0;
    sigact_pipe.sa_flags = 0;
	sigact_pipe.sa_flags |= SA_RESTART;
	sigact_pipe.sa_flags |= SA_RESTART;
	sigemptyset(&sigact_pipe.sa_mask);
    sigemptyset(&sigact_chld.sa_mask);
    sigaction(SIGCHLD, &sigact_chld, &old_sigact_chld);
	sigaction(SIGPIPE, &sigact_pipe, &old_sigact_pipe);
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	int cli_addr_len;
	int connfd;
	int conc_amnt = atoi(argv[3]);
	pid_t pid = getpid();
	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &srv_addr.sin_addr);
	srv_addr.sin_port = htons(atoi(argv[2]));

	for (int i = 0; i < conc_amnt - 1; i++) {
        if (fork() != 0) {
			int pin = i+1;
			char fn_res[20];
			pid = getpid();
			sprintf(fn_res, "stu_cli_res_%d.txt", pin);
        	fp_res = fopen(fn_res, "ab"); // Write only， append at the tail. Open or create a binary file;
			if(!fp_res){
				printf("[cli](%d) child exits, failed to open file \"stu_cli_res_%d.txt\"!\n", pid, pin);
				exit(-1);
			}
			bprintf(fp_res, "[cli](%d) child process %d is created!\n", pid, pin);
			connfd = socket(PF_INET, SOCK_STREAM, 0);
			while (1) {
				int res = connect(connfd, (struct sockaddr*) &srv_addr, sizeof(srv_addr));
				if (!res) {
					char ip_str[20]={0};
					bprintf(fp_res, "[cli](%d) server[%s:%d] is connected!\n", pid, \
						inet_ntop(AF_INET, &srv_addr.sin_addr, ip_str, sizeof(ip_str)), \
							ntohs(srv_addr.sin_port));
					echo_rqt(connfd, pin);
				}
				break;
			}
			close(connfd);
			bprintf(fp_res, "[cli](%d) connfd is closed!\n", pid);
			bprintf(fp_res, "[cli](%d) child process is going to exit!\n", pid);
			if(fp_res){
				if(!fclose(fp_res))
					printf("[cli](%d) stu_cli_res_%d.txt is closed!\n", pid, pin);
			}
			exit(1);
		} else {
			close(connfd);
		}	
	}
	char fn_res[20];
	sprintf(fn_res, "stu_cli_res_%d.txt", 0);
    fp_res = fopen(fn_res, "wb");
	if (!fp_res) {
		printf("[cli](%d) child exits, failed to open file \"stu_cli_res_0.txt\"!\n", pid);
		exit(-1);
	}
	//creat client fd
	connfd = socket(PF_INET, SOCK_STREAM, 0);
	//connect client to server
	while (1) {
		int res = connect(connfd, (struct sockaddr*) &srv_addr, sizeof(srv_addr));
		if(!res){
			char ip_str[20]={0};
			bprintf(fp_res, "[cli](%d) server[%s:%d] is connected!\n", pid, inet_ntop(AF_INET, &srv_addr.sin_addr, ip_str, sizeof(ip_str)), ntohs(srv_addr.sin_port));
			echo_rqt(connfd, 0);
		}
		break;
	}

	close(connfd);
	bprintf(fp_res, "[cli](%d) connfd is closed!\n", pid);
	bprintf(fp_res, "[cli](%d) parent process is going to exit!\n", pid);

	if(!fclose(fp_res)) printf("[cli](%d) stu_cli_res_0.txt is closed!\n", pid);
		
	return 0;
}