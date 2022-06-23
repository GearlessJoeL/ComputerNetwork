#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <assert.h>

#define BACKLOG 1024
#define bprintf(fp, format, ...) \
	if(fp == NULL){printf(format, ##__VA_ARGS__);} 	\
	else{printf(format, ##__VA_ARGS__);	\
			fprintf(fp, format, ##__VA_ARGS__);fflush(fp);}

int sig_type = 0, sig_to_exit = 0;
FILE * fp_res = NULL;

void sig_int (int signum) {	
	sig_type = signum;
	pid_t pid = getpid();
	bprintf(fp_res, "[srv](%d) SIGINT is coming!\n", pid);
    sig_to_exit = 1;
}

void sig_pipe (int signum) {	
	sig_type = signum;
	pid_t pid = getpid();
	bprintf(fp_res, "[srv](%d) SIGPIPE is coming!\n", pid);
}

void sig_chld (int signum) {
    sig_type = signum;
	pid_t pid = getpid();
	bprintf(fp_res, "[srv](%d) SIGCHLD is coming!\n", pid);
}

int install_sig_handlers(){
	int res = -1;
	struct sigaction sigact_pipe, old_sigact_pipe;
	sigact_pipe.sa_handler = sig_pipe;
	sigact_pipe.sa_flags = 0;
	sigact_pipe.sa_flags |= SA_RESTART;
	sigemptyset(&sigact_pipe.sa_mask);
	if (sigaction(SIGPIPE, &sigact_pipe, &old_sigact_pipe)) return -1;
	struct sigaction sigact_chld, old_sigact_chld;
	sigact_chld.sa_handler = &sig_chld;
    sigact_pipe.sa_flags = 0;
	sigact_pipe.sa_flags |= SA_RESTART;
	sigemptyset(&sigact_chld.sa_mask);
	if(sigaction(SIGCHLD, &sigact_chld, &old_sigact_chld)) return -2;
	struct sigaction sigact_int, old_sigact_int;
    sigemptyset(&sigact_int.sa_mask);
    sigact_int.sa_flags = 0;
	sigact_int.sa_handler = &sig_int;
	if(sigaction(SIGINT, &sigact_int, &old_sigact_int)) return -3;
	return 0;
}

int echo_rep(int sockfd){
	int len_h = -1, len_n = -1;
	int pin_h = -1, pin_n = -1;
	int res = 0;
	char *buf = NULL;
	pid_t pid = getpid();
    while (1){
		while (1){
			res = read(sockfd, &pin_n, sizeof(pin_n));
			if(res < 0){
				bprintf(fp_res, "[srv](%d) read pin_n return %d and errno is %d!\n", pid, res, errno);
				if(errno == EINTR){
					if(sig_type == SIGINT) return pin_h;
					continue;
				}
				return pin_h;
			}
			if(!res) return pin_h;
			pin_h = ntohl(pin_n);
			break;				
		}

		while (1){
			res = read(sockfd, &len_n, sizeof(len_n));
			if(res < 0){
				bprintf(fp_res, "[srv](%d) read len_n return %d and errno is %d\n", pid, res, errno);
				if(errno == EINTR){
					if(sig_type == SIGINT) return pin_h;
					continue;
				}
				return pin_h;
			}
			if(!res) return pin_h;
			len_h = ntohl(len_n);
			break;
		}
		int read_amnt = 0, len_to_read = len_h;
		buf = (char*)malloc(len_h * sizeof(char)+8);
		while (1){
			res = read(sockfd, buf+8+read_amnt, len_to_read);
			if(res < 0){
				bprintf(fp_res, "[srv](%d) read data return %d and errno is %d,\n", pid, res, errno);
				if(errno == EINTR){
					if(sig_type == SIGINT){
						free(buf);
						return pin_h;
					}
					continue;
				}
				free(buf);
				return pin_h;
			}
			if(!res){
				free(buf);
				return pin_h;
			}
			read_amnt += res;
			if(read_amnt == len_h){
				break;
			} else if (read_amnt < len_h){
				len_to_read = len_h - read_amnt;
			} else {
				free(buf);
				return pin_h;
			}
		}
		bprintf(fp_res, "[echo_rqt](%d) %s\n", pid, buf+8);
		memcpy(buf, &pin_n, 4);
		memcpy(buf+4, &len_n, 4);
		write(sockfd, buf, len_h+8);
        free(buf);
    }
	return pin_h;
}

int main(int argc, char* argv[]){
	if (argc != 3){
		printf("Usage:%s <IP> <PORT>\n", argv[0]);
		return -1;
	}
	pid_t pid = getpid();
	char ip_str[20]={0};
	char fn_res[20]={0};
	int res = -1;
	res = install_sig_handlers();
	if (res){
		printf("[srv](%d) parent exit failed to install signal handlers!\n", pid);
		return res;
	}
	fp_res = fopen("stu_srv_res_p.txt", "wb");
	if(!fp_res){
		printf("[srv](%d) failed to open file \"stu_srv_res_p.txt\"!\n", pid);
		return res;
	}
	struct sockaddr_in srv_addr, cli_addr;
	socklen_t cli_addr_len;
	int listenfd, connfd;
		/* IP地址转换推荐使用inet_pton()；端口地址转换推荐使用atoi(); */
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	srv_addr.sin_port = htons(atoi(argv[2]));
	// 按题设要求打印服务器端地址server[ip:port]，推荐使用inet_ntop();
	inet_ntop(AF_INET, &srv_addr.sin_addr, ip_str, sizeof(ip_str));
	bprintf(fp_res, "[srv](%d) server[%s:%d] is initializing!\n", pid, ip_str, (int)ntohs(srv_addr.sin_port));
	// 获取Socket监听描述符: listenfd = socket(x,x,x);
	listenfd = socket(PF_INET, SOCK_STREAM, 0);	
	if(listenfd == -1) return listenfd;
	// 绑定服务器Socket地址: res = bind(x,x,x);
	res = bind(listenfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	if(res) return res;
	// 开启服务监听: res = listen(x,x); TODO: uftrace只记录了listen()，很奇怪，不知是否有返回值；
	res = -9;
	res = listen(listenfd,BACKLOG);
	if(res)
		printf("[srv](%d) listen() returned %d\n", pid, res);
	else if(res == 0)
		printf("[srv](%d) listen() returned 0\n",pid);
    // 开启accpet()主循环，直至sig_to_exit指示服务器退出；
	while(!sig_to_exit) {
		// 获取cli_addr长度，执行accept()：connfd = accept(x,x,x);
		cli_addr_len = sizeof(cli_addr);
		// 以下代码紧跟accept()，用于判断accpet()是否因SIG_INT信号退出（本案例中只关心SIGINT）；也可以不做此判断，直接执行 connfd<0 时continue，因为此时sig_to_exit已经指明需要退出accept()主循环，两种方式二选一即可。
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_addr_len);
		if(connfd == -1 && errno == EINTR){
			if(sig_type == SIGINT)
				break;
			continue;
		}
		// 按题设要求打印客户端端地址client[ip:port]，推荐使用inet_ntop();
		inet_ntop(AF_INET, &cli_addr.sin_addr, ip_str, sizeof(ip_str));
		bprintf(fp_res, "[srv](%d) client[%s:%d] is accepted!\n", pid, ip_str, (int)ntohs(cli_addr.sin_port));
		fflush(fp_res);
		// 派生子进程对接客户端开展业务交互
		if(!fork()){
			// 获取当前子进程PID,用于后续子进程信息打印
			pid = getpid();
			// 打开res文件，首先基于PID命名，随后在子进程退出前再根据echo_rep()返回的PIN值对文件更名；
			sprintf(fn_res, "stu_srv_res_%d.txt", pid);
			fp_res = fopen(fn_res, "wb");// Write only， append at the tail. Open or create a binary file;
			if(!fp_res){
				printf("[srv](%d) child exits, failed to open file \"stu_srv_res_%d.txt\"!\n", pid, pid);
				exit(-1);
			}
			bprintf(fp_res, "[srv](%d) child process is created!\n", pid);
			// 关闭监听描述符
			close(listenfd);
			bprintf(fp_res, "[srv](%d) listenfd is closed!\n", pid);
			// 执行业务函数（返回客户端PIN）
			int pin = echo_rep(connfd);
			if(pin < 0){
				bprintf(fp_res, "[srv](%d) child exits, client PIN returned by echo_rqt() error!\n", pid);
				exit(-1);
			}
			// 更名子进程res文件（PIN替换PID）
			char fn_res_n[20]={0};
			sprintf(fn_res_n, "stu_srv_res_%d.txt", pin);
			if(!rename(fn_res, fn_res_n)){
				bprintf(fp_res, "[srv](%d) res file rename done!\n", pid);
			} else{			
				bprintf(fp_res, "[srv](%d) child exits, res file rename failed!\n", pid);
			}
			// 关闭连接描述符
			close(connfd);
			bprintf(fp_res, "[srv](%d) connfd is closed!\n", pid);
			bprintf(fp_res, "[srv](%d) child process is going to exit!\n", pid);
			// 关闭子进程res文件
			if(!fclose(fp_res)) printf("[srv](%d) stu_srv_res_%d.txt is closed!\n", pid, pin);
			exit(1);
		} else {	
			close(connfd);// 关闭连接描述符
			continue;// 继续accept()，处理下一个请求
		}
	}
	// 关闭监听描述符
	close(listenfd);
	bprintf(fp_res, "[srv](%d) listenfd is closed!\n", pid);
	bprintf(fp_res, "[srv](%d) parent process is going to exit!\n", pid);
	// 关闭父进程res文件
	if(!fclose(fp_res)) printf("[srv](%d) stu_srv_res_p.txt is closed!\n", pid);
	return 0;
}