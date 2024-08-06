#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h> // for close()
#include <cerrno> // for errno
#include <cstring> // for strerror()
#include <iostream>
#include <cstdlib>

using namespace std;

#define PORT 8082
#define MAX_EVENTS 1024
#define BUFFER_SIZE 20480
#define BUFFER_SIZE1 1024
#define HEADER_SIZE 16

void error_handling(const char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void extractFilename(const char* buffer, char* buffer1) {
    // Find the position of the '$' character
    const char* delimiter = strchr(buffer, '$');

    // Check if '$' was found
    if (delimiter != NULL) {
        // Calculate the length of the filename
        size_t filenameLength = delimiter - buffer;

        // Construct the new string with "GETFILE " prefix
        snprintf(buffer1, filenameLength + 9, "GETFILE %.*s", (int)filenameLength, buffer);
    }
    else {
        // Handle case where no '$' is found
        snprintf(buffer1, 8, "GETFILE ");
    }
}
string getstr(const std::string& input) {
    std::string keyword = "GETFILE ";
    std::string delimiter = "$";

    // 找到 keyword 的位置
    size_t keywordPos = input.find(keyword);
    if (keywordPos != std::string::npos) {
        // 获取 keyword 后面的部分
        size_t startPos = keywordPos + keyword.length();

        // 找到 delimiter 的位置
        size_t endPos = input.find(delimiter, startPos);
        if (endPos != std::string::npos) {
            // 提取 xxxxxx 部分
            return input.substr(startPos, endPos - startPos);
        }
    }
    // 如果未找到 keyword 或 delimiter，返回空字符串
    return "";
}
void send_file(const std::string& filename, int sockfd) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error opening file: " << filename << "\n";
        return;
    }

    // 获取文件大小
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // 创建缓冲区
    std::vector<char> buffer(BUFFER_SIZE);

    // 读取文件并发送数据
    while (fileSize > 0) {
        // 确定读取的字节数
        std::streamsize bytesToRead = std::min(static_cast<std::streamsize>(BUFFER_SIZE), fileSize);

        // 从文件读取数据
        file.read(buffer.data(), bytesToRead);
        if (!file) { 
            std::cerr << "Error reading file\n";
            return;
        }

        // 发送读取的数据
        ssize_t bytesSent = send(sockfd, buffer.data(), bytesToRead, 0);
        if (bytesSent == -1) {
            std::cerr << "Error sending data: " << strerror(errno) << "\n";
            return;
        }
        else if (bytesSent != bytesToRead) {
            std::cerr << "Warning: Not all data sent\n";
            return;
        }

        // 更新剩余文件大小
        fileSize -= bytesSent;
    }

    if (file.bad()) {
        std::cerr << "Error reading file\n";
    }

    file.close();
}
int count = 0;//记录
pthread_mutex_t buffer_mutex;
int main() {
    //显示本机的地址和状态
    std::cout << "IP Addresses:" << std::endl;
    std::system("hostname -I"); // 简单获取IP地址
    std::cout << "Network Status:" << std::endl;
    std::system("netstat -tuln"); // 使用netstat显示网络状态
    //主程序
    pthread_mutex_init(&buffer_mutex, NULL);
    int server_fd, new_socket, epoll_fd, event_count;
    struct sockaddr_in address;
    struct epoll_event event, events[MAX_EVENTS];
    char buffer[BUFFER_SIZE];
    char buffer1[10];//用来记录文件名称
    int client_count = 0;
    int client_fds[MAX_EVENTS];  // 用于存储客户端文件描述符
    memset(client_fds, 0, sizeof(client_fds));  // 初始化客户端文件描述符数组
    char fileName[16];//记录文件名

    // 创建服务器套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error_handling("socket failed");
    }

    // 设置地址复用
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        error_handling("setsockopt");
    }

    // 绑定套接字
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        error_handling("bind failed");
    }

    // 监听套接字
    if (listen(server_fd, 3) < 0) {
        error_handling("listen");
    }

    // 创建 epoll 实例
    if ((epoll_fd = epoll_create1(0)) < 0) {
        error_handling("epoll_create1");
    }

    // 将服务器套接字添加到 epoll 实例
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
        error_handling("epoll_ctl: server_fd");
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // 等待事件发生
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count < 0) {
            error_handling("epoll_wait");
        }

        for (int i = 0; i < event_count; i++) {
            if (events[i].data.fd == server_fd) {
                // 处理新连接
                socklen_t addrlen = sizeof(address);
                if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
                    error_handling("accept");
                }

                printf("New connection: fd %d, ip %s, port %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                client_count++;
                printf("Total connected clients: %d\n", client_count);

                // 将新连接添加到 epoll 实例
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = new_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event) < 0) {
                    error_handling("epoll_ctl: new_socket");
                }

                // 将新连接的文件描述符添加到客户端数组
                for (int j = 0; j < MAX_EVENTS; j++) {
                    if (client_fds[j] == 0) {
                        client_fds[j] = new_socket;
                        break;
                    }
                }
            }
            else {
                // 处理现有连接上的数据
                memset(buffer, 0, BUFFER_SIZE);
                int addrlen;
                int client_fd = events[i].data.fd;
                //pthread_mutex_lock(&buffer_mutex); // Lock the mutex
                int bytes_read = read(client_fd, buffer, BUFFER_SIZE);
                //pthread_mutex_unlock(&buffer_mutex); // Unlock the mutex
                printf("filename: %s\n", buffer);
                
                if (bytes_read == 0) {
                    // 处理断开连接
                    getpeername(client_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    printf("Client disconnected: ip %s, port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(client_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                    client_count--;
                    printf("Total connected clients: %d\n", client_count);

                    // 从客户端数组中删除文件描述符
                    for (int j = 0; j < client_count; j++) {
                        if (client_fds[j] == client_fd) {
                            client_fds[j] = 0;
                            break;
                        }
                    }
                }
                else if (bytes_read > 0) {
                    //发送数据给所有客户端
                    //buffer[bytes_read] = '\0';
                    char message_type[9] = { 0 };
                    strncpy(message_type, buffer, 8);

                    //如果是文件，则下载到本地
                    
                    if (strcmp(message_type, "TEXTMSG ") != 0 && strcmp(message_type, "GETFILE ") != 0) {   
                        //printf("filename: %s\n", buffer);
                        memset(fileName, 0, 16);
                        int i = 0;
                        for (; i < 8; i++) {
                            if (buffer[i] != '$') {
                                fileName[i] = buffer[i];
                            }
                            else {
                                break;
                            }
                        }
                        printf("%s\n", fileName);

                        //解析文件并保存到本地
                        if (bytes_read <= i) {
                            buffer[0] = '\0';  // 如果 n 大于或等于字符串长度，清空字符串
                        }
                        else { 
                            memmove(buffer, buffer + (i + 1), bytes_read - (i + 1));  // 使用 memmove 来移动数据
                        }
                        FILE* fp = fopen(fileName, "wb");
                        fwrite(buffer, 1, bytes_read - (i + 1), fp);
                        fclose(fp);
                    }                  
                    for (int j = 0; j < client_count+1; j++) {
                        if (client_fds[j] != 0 && client_fds[j] != client_fd && strcmp(message_type, "TEXTMSG ") == 0) {
                            //处理文本消息  发送给除了发送端之外的所有客户端
                            send(client_fds[j], buffer, bytes_read, 0);
                            printf("Received message from client %d: %s\n", client_fd, buffer);
                        }
                        else if (client_fds[j] != 0 && client_fds[j] == client_fd && strcmp(message_type, "GETFILE ")==0) {                            
                            //处理请求下载  发送给请求的客户端
                            printf("-------------%s\n", buffer);                            
                            /*char fileName[8];
                            int i = 8, j = 0;
                            for (; i < bytes_read; i++,j++) {
                                if (buffer[i] != '$') {
                                    fileName[j] = buffer[i];
                                }else {
                                    break;
                                }
                            } 
                            char fileName1[j];                            
                            strncpy(fileName1, fileName, j);
                            printf("%s1\n", fileName1);*/
                            string fileName1 = getstr(buffer);
                            cout << fileName1 << endl;
                            char filepath[256];
                            /*FILE* fp = fopen("1.jpg", "rb");
                            if (fp == NULL) {
                                fprintf(stderr, "Error: Failed to open the file '%s': %s\n", filepath, strerror(errno));
                            }
                            else {
                                printf("open file sucess\n");
                            }*/ 
                            send_file(fileName1, client_fd);
                            //memset(fileName, 0, j);
                            //fclose(fp);                                
                        }
                        else if (client_fds[j] != 0 && strcmp(message_type, "TEXTMSG ") != 0 && strcmp(message_type, "GETFILE ") != 0) {
                            
                            //将文件名发送给所有客户端
                           //extractFilename(buffer, buffer1);
                            printf("aaaa--%s\n", fileName);
                            string s1 = "GETFILE ";
                            string s2(fileName);
                            string s = s1 + s2;
                            
                            if (send(client_fds[j], s.c_str(), s.length(), 0) < 0) {
                                perror("Failed to send to client");
                            }
                        
                        }
                    }

                }
            }
        }
    }

    close(server_fd);
    return 0;
}
