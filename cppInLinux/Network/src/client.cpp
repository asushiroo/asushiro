#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>



class Client {
public:
    Client(const char* ip, int port) {
        // create socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(port);
        client_addr.sin_addr.s_addr = inet_addr(ip);
    }

    void send_intreval() {
        // connect to server
        if (connect(sockfd, (struct sockaddr*) &client_addr, sizeof(client_addr)) < 0) {
            perror("connection failed");
            exit(EXIT_FAILURE);
        } else {
            while (true) {
                // send message
                std::string msg;
                std::cout << "Enter message (or exit): ";
                std::getline(std::cin, msg);

                if (msg == "exit") break;

                // 发送数据
                if (send(sockfd, msg.c_str(), msg.size(), 0) < 0) {
                    perror("send error");
                    break;
                }

                char buf[1024];
                int n = recv(sockfd, buf, sizeof(buf) - 1, 0);
                if (n <= 0) {
                    std::cout << "Server disconnected" << std::endl;
                    break;
                }

                buf[n] = '\0';
                std::cout << "Server reply: " << buf << std::endl;
            }
        }
    }
protected:
    int sockfd;
    struct sockaddr_in client_addr;
};


int main() {
    Client client("127.0.0.1", 1145);
    client.send_intreval();
    return 0;
}