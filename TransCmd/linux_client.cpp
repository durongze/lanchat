#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>


#define CAMERA_WIDTH 320
#define CAMERA_HEIGHT 240
#define BITMAPINFOHEADER_SIZE 40

// #define DEF_ADDR INADDR_ANY
#define DEF_ADDR "192.168.137.1"
#define DEF_PORT 3000

typedef struct {
    int size; // 要保持一致
    char buf[CAMERA_WIDTH * CAMERA_HEIGHT * 4 + BITMAPINFOHEADER_SIZE];
}TcpPackage;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << argv[0] << " < cmd > " << std::endl;
        return -1;
    }
    int domain = AF_INET;
    int type = SOCK_STREAM;
    int protocol = IPPROTO_TCP;

    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_addr.s_addr = inet_addr(DEF_ADDR);
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(DEF_PORT);
    struct sockaddr *addr = (struct sockaddr*)&servAddr;
    socklen_t addrlen  = sizeof(servAddr);

    TcpPackage img = {sizeof(TcpPackage) - 4, {0}};
    strncpy(img.buf, argv[1], strlen(argv[1]));

    int sockfd = socket(domain, type, protocol);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    int ret = connect(sockfd, addr, addrlen);
    if (ret < 0) {
        perror("connect");
        return -1;
    }
    ret = send(sockfd, (char*)&img, sizeof(img), NULL);
    if (ret <= 0) {
        perror("send");
        // return -1;
    }
    ret = recv(sockfd, (char*)&img, sizeof(img), 0);
    std::cout << "msg:" << img.buf << std::endl;
    close(sockfd);
    return 0;
}
