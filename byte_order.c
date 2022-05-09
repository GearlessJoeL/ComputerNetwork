#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>


int main (int argc, char* argv[]){
    char* ipv4 = argv[1];
    char* port = argv[2];
    int cnt = 3;
    u_int32_t ipv4_h = 0, ipv4_n = 0;
    u_int16_t port_h = 0, port_n = 0;
    int now = 0;
    int aipv4_h[4];
    for (int i = 0; i < strlen(ipv4); i ++){
        if (ipv4[i] == '.'){
            ipv4_n <<= 8;
            ipv4_n += now;
            aipv4_h[cnt--] = now;
            now = 0;
        } else {
            now = now * 10 + ipv4[i] - '0';
        }
    }
    aipv4_h[0] = now;
    ipv4_n <<= 8;
    ipv4_n += now;
    ipv4_h = ntohl(ipv4_n);
    port_n = atoi(port);
    port_h = ntohs(port_n);
    printf("[ipv4_n_cbc_d]%s\n", ipv4);
    printf("[ipv4_n_cbc_x]%08X\n", ipv4_n);
    printf("[ipv4_n_asw_x]%08X\n", ipv4_h);
    printf("[port_n_cbc_x]%04X\n", port_n);
    printf("[port_n_asw_x]%04X\n", port_h);
    printf("[port_n_asw_d]%d\n", port_h);
    printf("[ipv4_h_cbc_d]%d.%d.%d.%d\n", aipv4_h[0], aipv4_h[1], aipv4_h[2], aipv4_h[3]);
    printf("[ipv4_h_cbc_x]%08X\n", ipv4_h);
    printf("[ipv4_h_asw_x]%08X\n", ipv4_n);
    printf("[port_h_cbc_x]%04X\n", port_h);
    printf("[port_h_asw_x]%04X\n", port_n);
    printf("[port_h_asw_d]%d\n", port_n);
    return 0;
}