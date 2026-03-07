#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8000
#define WEB_PORT 8002
#define BUF_SIZE 1024

int device_fd = -1;
int web_fd = -1;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

float current_temp = 0.0;
float current_hum = 0.0;
// 发送数据到Web
void send_to_web(const char* data) {
    pthread_mutex_lock(&lock);
    if (web_fd != -1) {
        // 直接转发单片机原始数据
        send(web_fd, data, strlen(data), 0);
        printf("Sent to Web: %s", data);
    }
    pthread_mutex_unlock(&lock);
}
// 解析设备数据
void parse_device_data(const char* data) {
    // 解析温度数据
    if (strstr(data, "Tem:") != NULL) {
        // 修正解析格式：包含换行符
        if (sscanf(data, "Tem: %fC\n", &current_temp) == 1) {
            printf("Parsed temperature: %.1f°C\n", current_temp);
        }
        if(sscanf(data,"Hum:%fC\n",&current_hum)==1){
            print("Parsed humadity:%.1f\n",current_hum);
        }
    }
}

// 接收线程
void *recv_thread(void *arg) {
    int client_fd = *(int *)arg;
    char buf[BUF_SIZE];
    ssize_t n;

    while ((n = recv(client_fd, buf, BUF_SIZE - 1, 0)) > 0) {
        buf[n] = '\0';
        printf("Device: %s", buf);

        // 解析数据
        parse_device_data(buf);

        // 直接转发原始数据到Web
        send_to_web(buf);
    }

    pthread_exit(NULL);
}
// 初始化Web服务器
int init_web_server() {
    int web_server_fd;
    struct sockaddr_in web_addr;

    web_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (web_server_fd < 0) {
        perror("web socket");
        return -1;
    }

    int opt = 1;
    setsockopt(web_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&web_addr, 0, sizeof(web_addr));
    web_addr.sin_family = AF_INET;
    web_addr.sin_addr.s_addr = INADDR_ANY;
    web_addr.sin_port = htons(WEB_PORT);

    if (bind(web_server_fd, (struct sockaddr*)&web_addr, sizeof(web_addr)) < 0) {
        perror("web bind");
        close(web_server_fd);
        return -1;
    }

    if (listen(web_server_fd, 5) < 0) {
        perror("web listen");
        close(web_server_fd);
        return -1;
    } printf("Web server listening on port %d...\n", WEB_PORT);
    return web_server_fd;
}

int main() {
    int server_fd, web_server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // 创建单片机服务器
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // 配置单片机服务器
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(1);
    }
   if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    // 创建Web服务器
    web_server_fd = init_web_server();
    if (web_server_fd < 0) {
        close(server_fd);
        exit(1);
    }

    // 接受设备连接
    device_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (device_fd < 0) {
        perror("accept");
        close(server_fd);
        close(web_server_fd);
        exit(1);
    }
    printf("Device connected: %s:%d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  // 接受Web连接
    web_fd = accept(web_server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (web_fd < 0) {
        perror("web accept");
        close(device_fd);
        close(server_fd);
        close(web_server_fd);
        exit(1);
    }
    printf("Web client connected: %s:%d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // 创建接收线程
    pthread_t tid_recv;
    pthread_create(&tid_recv, NULL, recv_thread, &device_fd);

    // 主循环
    while(1) {
        sleep(10);
    }

    close(device_fd);
    close(web_fd);
    close(server_fd);
    close(web_server_fd);
    return 0;
}