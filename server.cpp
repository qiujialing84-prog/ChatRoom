#include <iostream>    
#include <cstring>     
#include <cstdlib>     // exit
#include <unistd.h>    // close
#include <arpa/inet.h> // 网络地址转换
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>      // 线程
#include <vector>      

using namespace std; 

#define BUFFER_SIZE 1024
#define PORT 10086     

// 用 vector 管理所有客户端
vector<int> clientSocks;

// 接收消息 + 广播线程
void recv_func(int clientSock);

int main() {
    cout << "=== 群聊服务端已启动 ===" << endl;

    //创建服务端 socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == -1) {
        perror("socket 创建失败");
        exit(EXIT_FAILURE);
    }

    //绑定地址和端口
    struct sockaddr_in addr{}; 
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind 失败");
        exit(EXIT_FAILURE);
    }

    // 监听
    if (listen(serverSocket, 10) == -1) {
        perror("listen 失败");
        exit(EXIT_FAILURE);
    }

    //循环接受客户端连接
    while (true) {
        struct sockaddr_in cAddr{};
        socklen_t len = sizeof(cAddr);

        // 等待客户端连接
        int newClient = accept(serverSocket, (sockaddr*)&cAddr, &len);
        if (newClient == -1) {
            perror("accept 失败");
            continue;
        }

        // 加入客户端列表
        clientSocks.push_back(newClient);
        int idx = clientSocks.size();

        cout << "客户端 " << idx << " 已连接！IP: " 
             << inet_ntoa(cAddr.sin_addr) 
             << " 端口: " << ntohs(cAddr.sin_port) 
             << endl;

        //创建独立线程处理这个客户端
        thread recvThread(recv_func, newClient);
        recvThread.detach(); // 分离线程
    }

    close(serverSocket);
    return 0;
}

// 接收消息 + 广播给所有其他客户端
void recv_func(int clientSock) {
    char recv_buff[BUFFER_SIZE];

    while (true) {
        memset(recv_buff, 0, BUFFER_SIZE);

        // 接收客户端消息
        int r = recv(clientSock, recv_buff, BUFFER_SIZE - 1, 0);

        // 客户端断开 / 读取失败
        if (r <= 0) {
            cout << "\n客户端已断开连接" << endl;

            // 从列表中移除
            for (auto it = clientSocks.begin(); it != clientSocks.end(); ++it) {
                if (*it == clientSock) {
                    clientSocks.erase(it);
                    break;
                }
            }
            close(clientSock);
            return;
        }

        // 打印收到的消息
        cout << "\n[客户端] " << recv_buff << endl;

        // 广播：发给所有在线客户端
        for (int sock : clientSocks) {
            if (sock != clientSock) { // 不发给自己
                send(sock, recv_buff, r, 0);
            }
        }

        // 输入 quit 退出
        if (strncmp(recv_buff, "quit", 4) == 0) {
            cout << "客户端主动退出聊天" << endl;

            for (auto it = clientSocks.begin(); it != clientSocks.end(); ++it) {
                if (*it == clientSock) {
                    clientSocks.erase(it);
                    break;
                }
            }
            close(clientSock);
            return;
        }
    }
}