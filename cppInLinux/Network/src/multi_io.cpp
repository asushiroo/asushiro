#include <sys/socket.h> //socket函数
#include <arpa/inet.h> //inet_addr函数
#include <netinet/in.h> //htons函数
#include <stdio.h> 
#include <cstring> //memset函数
#include <unistd.h> //close函数
#include <memory> //unique_ptr
#include <sys/poll.h> //poll函数
#include <sys/epoll.h> 


class MultiIO {
public:
    MultiIO(int port) {
        //建立socket链接
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket error");
            return;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
            perror("bind error");
            return;
        }

        //监听连接
        if (listen(sockfd, 10) == -1) {
            perror("listen error");
            return;
        } else {
            puts("listen success");
        }
    }
    virtual ~MultiIO() {
        close(sockfd);
    }

    virtual void run() = 0;
protected:
    int sockfd;
};


class SelectIO: public MultiIO {
public:
    explicit SelectIO(int port) : MultiIO(port) {}
    ~SelectIO() override {}

    void run() override {
           /*
        typedef sturct {
            unsigned long fds_bits[1024 / (8 * sizeof(long))]
        } _kernel_fd_set;
        默认的fds_bits数组大小为1024，所以最多可以监听1024个连接。
    */
        //初始化读写事件集
        fd_set rfds, rset;
        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        int maxfd = sockfd;

        while (true) {
            rset = rfds;

            //等待事件发生
            int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

            if (FD_ISSET(sockfd, &rset)) {
                //接收连接请求
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
                
                if (clientfd < 0) {
                    perror("accept error");
                    continue;
                } else {
                    printf("sockfd: %d, clientfd: %d\n", sockfd, clientfd);
                }
                FD_SET(clientfd, &rfds);
                if (clientfd > maxfd) {
                    maxfd = clientfd;
                }
            }

            //处理连接请求
            for (int i = sockfd + 1; i <= maxfd; ++ i) {
                if (FD_ISSET(i, &rset)) {
                    char buf[1024];
                    int count = recv(i, buf, sizeof(buf), 0);
                    
                    //客户端断开连接，读写事件集置0，关闭连接
                    if (count == 0) {
                        FD_CLR(i, &rfds);
                        close(i);
                        puts("client disconnected");
                    } else {
                        //转发数据
                        send(i, buf, count, 0);
                    }
                }
            }  
        }
    }
};


class PollIO: public MultiIO {
public:
    explicit PollIO(int port) : MultiIO(port) {}
    ~PollIO() override {}

    void run() override {
        //初始化poll事件集
        struct pollfd fds[1024] = {0};
        fds[sockfd].fd = sockfd;
        fds[sockfd].events = POLLIN;

        int maxfd = sockfd;
        while (true) {
            int nready = poll(fds, maxfd + 1, -1); 

            //接收连接请求
            if (fds[sockfd].revents & POLLIN) { 

                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);

                if (clientfd < 0) {
                    perror("accept error");
                    continue;
                } else {
                    printf("sockfd: %d, clientfd: %d\n", sockfd, clientfd);
                }

                //链接请求加入事件集
                fds[clientfd].fd = clientfd;
                fds[clientfd].events = POLLIN;
                if (clientfd > maxfd) {
                    maxfd = clientfd;
                }
            }

            //处理连接请求
            for (int i = sockfd + 1; i <= maxfd; ++ i) {
                if (fds[i].revents & POLLIN) {
                    char buf[1024];
                    int count = recv(i, buf, sizeof(buf), 0);

                    if (count == 0) {
                        fds[i].fd = -1;
                        fds[i].events = 0;
                        close(i);
                        puts("client disconnected");
                    } else {
                        send(i, buf, count, 0);
                    }
                }
            }
        }
    }
};


class EpollIO: public MultiIO {
public:
    explicit EpollIO(int port) : MultiIO(port) {}
    ~EpollIO() override {}

    void run() override {
        int epollfd = epoll_create(1024); //参数为了兼容旧的版本，新版已无作用
        struct epoll_event ev;
        // ev.events = EPOLLIN; // 默认为水平触发， 定义文件大小，实现分包读取
        ev.events = POLLIN | EPOLLET; // 边缘触发, 一次性全部读完

        ev.data.fd = sockfd;

        epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);

        struct epoll_event events[1024];
        while (true) {
            int nready = epoll_wait(epollfd, events, 1024, -1);

            for (int i = 0; i < nready; ++ i) {
                int connfd = events[i].data.fd;

                if (connfd == sockfd) {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);

                    if (clientfd < 0) {
                        perror("accept error");
                        continue;
                    } else {
                        printf("sockfd: %d, clientfd: %d\n", sockfd, clientfd);
                    }
                    ev.events = EPOLLIN;
                    ev.data.fd = clientfd;
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
                } 
                else if (events[i].events & EPOLLIN) {
                    char buf[10];
                    int count = recv(connfd, buf, sizeof(buf), 0);

                    if (count == 0) {
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, NULL);
                        close(connfd);
                        puts("client disconnected");
                    } else {
                        send(connfd, buf, count, 0);
                    }
                }
            }
        }
    }
};


enum class IOType {
    SELECT_IO,
    POLL_IO,
    EPOLL_IO
};

class IOFactory {
public:
    static std::unique_ptr<MultiIO> create(IOType type, int port) {
        switch (type) {
            case IOType::SELECT_IO:
                return std::make_unique<SelectIO>(port);

            case IOType::POLL_IO:
                return std::make_unique<PollIO>(port);

            case IOType::EPOLL_IO:
                return std::make_unique<EpollIO>(port);

            default:
                throw std::runtime_error("Unknown IOType");
        }
    }
};




int main() {
    auto server = IOFactory::create(IOType::EPOLL_IO, 1145);
    server->run();
    return 0;
}
