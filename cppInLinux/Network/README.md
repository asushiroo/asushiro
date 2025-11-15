# 1 TCP Server in C++
```cpp
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) { perror("socket"); return EXIT_FAILURE; }

    //允许服务器在端口处于 TIME_WAIT 状态时也能立即重新绑定端口（避免 EADDRINUSE 错误）
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(sockfd);
        return EXIT_FAILURE;
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(1145);

    if (bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("bind");
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (listen(sockfd, 10) == -1) {
        perror("listen");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("Listening on port 1145. Press Enter to exit.\n");
    getchar();

    close(sockfd);
    return 0;
}

```
- `#include <sys/socket.h>`
    套接字 API（socket, bind, listen, accept 等）的声明在这里或相关头里。
- `#include <errno.h>`
    提供 errno 全局变量，用于查看错误码（程序里没有直接使用 errno，但 perror 会根据它输出信息）。
- `#include <netinet/in.h>`
    提供 sockaddr_in 结构体、INADDR_ANY、端口/地址相关常量和 htons/htonl 的声明（通常可用）。

> int sockfd = socket(AF_INET, SOCK_STREAM, 0);
socket(domain, type, protocol) 创建一个套接字，返回文件描述符（若失败返回 -1）。
- AF_INET：IPv4 地址族（表示使用 IPv4）。
- SOCK_STREAM：面向连接的字节流套接字（即 TCP）。
- protocol = 0：让内核选择默认协议（对于 AF_INET + SOCK_STREAM 即 TCP）。

> `struct sockaddr_in serveraddr;`定义一个 IPv4 地址结构用于 bind。
`memset(&serveraddr, 0, sizeof(serveraddr));`把结构体清零，确保没有未初始化的字段（良好做法）。

> serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
 INADDR_ANY（通常值为 0.0.0.0）表示绑定到本机所有可用网络接口（即接受任何到达本机任一网卡的连接）。
 `htonl`：将 32-bit 主机字节序转换为网络字节序（大端）。这里将 INADDR_ANY 转为网络序并赋给 sin_addr.s_addr。`注意`：还有 `inet_pton` 用于把字符串 IP（例如 "127.0.0.1"）转为网络字节序的二进制形式。小提示：也常见写法 serveraddr.sin_addr.s_addr = INADDR_ANY;（某些平台会隐式接受），但显式 htonl 更严谨。

 >serveraddr.sin_port = htons(1145); //绑定端口
`htons` 将 16-bit 主机字节序转换为网络字节序。端口必须放入网络字节序

>`bind`() 将套接字与指定地址/端口绑定。失败时返回 -1 并设置 errno