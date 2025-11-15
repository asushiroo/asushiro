#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>


void *clientThread(void *arg);

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
    serveraddr.sin_family = AF_INET; // IPv4
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any IP address
    serveraddr.sin_port = htons(1145); // Port number

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
    
#if 0
    // Accept a connection
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len); //阻塞，等待有连接过来
    if (clientfd == -1) 
        perror("accept");
    char buffer[128] = {0};
    int len = recv(clientfd, buffer, sizeof(buffer), 0); //阻塞，接收数据，buffer的大小为128字节
    //recv返回0的时候就是对方调用了close，表示已经断开连接

    send(clientfd, "Hello, client!", len, 0); //发送数据，"Hello, client!"为要发送的消息
    
    printf("sockfd: %d, clientfd: %d, len: %d, buffer: %s\n", sockfd, clientfd, len, buffer); 
    //stdin: 0 stdout: 1 stderr: 2, 所以sockfd: 3, clientfd: 4

    close(clientfd);
#elif 0
    // Accept a connection
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len); //阻塞，等待有连接过来
    if (clientfd == -1) 
        perror("accept");
    while (1) {
        char buffer[128] = {0};
        int len = recv(clientfd, buffer, sizeof(buffer), 0); 
        if (len == 0) break; //对方调用了close，表示已经断开连接

        send(clientfd, "Hello, client!", len, 0);
        
        printf("sockfd: %d, clientfd: %d, len: %d, buffer: %s\n", sockfd, clientfd, len, buffer); 
    }
    close(clientfd);
#else 
    while(1) {
        // Accept a connection
        struct sockaddr_in clientaddr;
        socklen_t len = sizeof(clientaddr);
        int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len); //阻塞，等待有连接过来
        if (clientfd == -1) {
            perror("accept");
            continue;
        }
        
        int *pclientfd = new int(clientfd); //将clientfd的地址存入一个指针变量中，以便在主线程中使用

        pthread_t tid;
        pthread_create(&tid, NULL, clientThread, pclientfd);
        pthread_detach(tid);
    }
#endif

    close(sockfd);
    return 0;
}

void *clientThread(void *arg) {
    int clientfd = *(int*)arg; //强制将arg转换成int指针，然后在解地址得到clientfd内容
    delete (int*)arg;  //在线程里释放

    while (1) {
        char buffer[128] = {0};
        int len = recv(clientfd, buffer, sizeof(buffer), 0); 
        if (len == 0) break; //对方调用了close，表示已经断开连接

        send(clientfd, "Hello, client!", len, 0);
        
        printf("clientfd: %d, len: %d, buffer: %s\n", clientfd, len, buffer); 
    }
    
    close(clientfd);
    return nullptr;
}
