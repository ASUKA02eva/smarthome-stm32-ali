#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8000        // 单片机连接端口
#define GUI_PORT 8001    // Python GUI连接端口
#define BUF_SIZE 1024

int device_fd = -1;      // 单片机连接文件描述符
int gui_fd = -1;         // GUI连接文件描述符
pthread_mutex_t gui_lock = PTHREAD_MUTEX_INITIALIZER;  // GUI连接互斥锁

float current_temp = 0.0;
float current_hum = 0.0;

// 发送数据到Python GUI
void send_to_gui(const char* data) {
    pthread_mutex_lock(&gui_lock);
    if (gui_fd != -1) {
        send(gui_fd, data, strlen(data), 0);
        printf("Sent to GUI: %s", data);
    }
    pthread_mutex_unlock(&gui_lock);
}

// 解析设备数据（修正了原逻辑错误）
void parse_device_data(const char* data) {
    // 解析温度数据
    if (strstr(data, "Tem:") != NULL) {
        if (sscanf(data, "Tem: %fC\n", &current_temp) == 1) {
            printf("Parsed temperature: %.1f°C\n", current_temp);
        }
    }
    // 解析湿度数据（独立判断，修正格式）
    if (strstr(data, "Hum:") != NULL) {
        if (sscanf(data, "Hum: %f\n", &current_hum) == 1) {
            printf("Parsed humidity: %.1f\n", current_hum);
        }
    }
}

// 接收单片机数据的线程
void *recv_thread(void *arg) {
    int client_fd = *(int *)arg;
    char buf[BUF_SIZE];
    ssize_t n;

    while ((n = recv(client_fd, buf, BUF_SIZE - 1, 0)) > 0) {
        buf[n] = '\0';
        printf("Device: %s", buf);

        // 解析数据
        parse_device_data(buf);

        // 转发原始数据到Python GUI
        send_to_gui(buf);
    }

    pthread_exit(NULL);
}

// 接收Python GUI数据的线程
void *gui_recv_thread(void *arg) {
    char buf[BUF_SIZE];
    ssize_t n;

    while ((n = recv(gui_fd, buf, BUF_SIZE - 1, 0)) > 0) {
        buf[n] = '\0';
        printf("Received from GUI: %s", buf);

        // 将GUI命令转发给单片机
        if (device_fd != -1) {
            send(device_fd, buf, strlen(buf), 0);
            printf("Sent to Device: %s", buf);
        }
    }

    // GUI断开连接处理
    pthread_mutex_lock(&gui_lock);
    gui_fd = -1;
    pthread_mutex_unlock(&gui_lock);
    printf("GUI disconnected\n");
    pthread_exit(NULL);
}

// 初始化GUI服务器
int init_gui_server() {
    int gui_server_fd;
    struct sockaddr_in gui_addr;

    gui_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (gui_server_fd < 0) {
        perror("gui socket");
        return -1;
    }

    int opt = 1;
    setsockopt(gui_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&gui_addr, 0, sizeof(gui_addr));
    gui_addr.sin_family = AF_INET;
    gui_addr.sin_addr.s_addr = INADDR_ANY;
    gui_addr.sin_port = htons(GUI_PORT);

    if (bind(gui_server_fd, (struct sockaddr*)&gui_addr, sizeof(gui_addr)) < 0) {
        perror("gui bind");
        close(gui_server_fd);
        return -1;
    }

    if (listen(gui_server_fd, 5) < 0) {
        perror("gui listen");
        close(gui_server_fd);
        return -1;
    }
    printf("GUI server listening on port %d...\n", GUI_PORT);
    return gui_server_fd;
}

int main() {
    int server_fd, gui_server_fd;
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

    printf("Device server listening on port %d...\n", PORT);

    // 初始化GUI服务器
    gui_server_fd = init_gui_server();
    if (gui_server_fd < 0) {
        close(server_fd);
        exit(1);
    }

    // 接受设备连接
    device_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (device_fd < 0) {
        perror("accept device");
        close(server_fd);
        close(gui_server_fd);
        exit(1);
    }
    printf("Device connected: %s:%d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // 接受GUI连接
    gui_fd = accept(gui_server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (gui_fd < 0) {
        perror("accept gui");
        close(device_fd);
        close(server_fd);
        close(gui_server_fd);
        exit(1);
    }
    printf("GUI connected: %s:%d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // 创建接收单片机数据的线程
    pthread_t tid_recv;
    pthread_create(&tid_recv, NULL, recv_thread, &device_fd);

    // 创建接收GUI数据的线程
    pthread_t tid_gui_recv;
    pthread_create(&tid_gui_recv, NULL, gui_recv_thread, NULL);

    // 主循环
    while(1) {
        sleep(10);
    }

    close(device_fd);
    close(gui_fd);
    close(server_fd);
    close(gui_server_fd);
    return 0;
}