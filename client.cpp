#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>

using namespace std;

#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define PORT 10086

int sock;

// 接收服务端广播消息
void recv_thread() {
    char buf[BUFFER_SIZE];
    while (true) {
        memset(buf, 0, BUFFER_SIZE);
        int len = recv(sock, buf, BUFFER_SIZE, 0);
        if (len <= 0) {
            cout << "\n服务端断开连接" << endl;
            close(sock);
            exit(0);
        }
        cout << "\n[群消息] " << buf << endl;
        cout << "输入消息：" << flush;
    }
}

int main() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        cout << "连接服务端失败" << endl;
        return -1;
    }

    cout << "=== 成功进入群聊 ===" << endl;
    thread t(recv_thread);
    t.detach();

    char buf[BUFFER_SIZE];
    while (true) {
        cout << "输入消息：" << flush;
        cin.getline(buf, BUFFER_SIZE);
        send(sock, buf, strlen(buf), 0);

        if (strcmp(buf, "quit") == 0) {
            break;
        }
    }

    close(sock);
    return 0;
}