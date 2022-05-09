#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char* argv[]){
    int len = strlen(argv[1]);
    char *post;
    if (len % 4){
        char temp[4-(len%4)];
        len += 4-(len%4);
        post = (char*)malloc((len)*sizeof(char));
        memset(temp, '0', sizeof(temp));
        post = strcat(argv[1], temp);
    } else {
        post = (char*)malloc((len)*sizeof(char));
        post = argv[1];
    }
    int checksum = 0;
    for (int i = 0; i < len/4; i ++){
        int num = 0;
        for (int j = 0; j < 4; j ++){
            char now = post[i*4+j];
            if (now <= '9'){
                num = num * 16 + now - '0';
            } else {
                num = num * 16 + now - 'a' + 10;
            }
        }
        checksum += num;
    }
    checksum += (checksum >> 16);
    checksum &= 0xffff;
    printf("%04x\n", (~checksum)&0xffff);
    return 0;
}