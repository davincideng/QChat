# QQ聊天室（QT、Linux、epoll）

环境    服务端  Ubuntu18.04 LTS
            客户端Qt Creator5.02    Kit：MinGW_32_bit

## 服务端

ip地址：初始设为127.0.0.1代表本机网卡的ip地址，
		0.0.0.0代表任何Ip地址。

端口：初始设为8082



运行命令：

1. ./Qt_server.out   启动服务端
2. ls -l查看文件信息  客户端发送的文件默认存放在此文件夹下



### 数据包头的设计：

```
client---->server
文本消息：“TEXTMSG ”
文件：文件名称
请求下载："GETFILE "

server---->client
文本消息：“TEXTMSG ”
文件： 无
请求下载："GETFILE "
```



### 实现流程：

1. 创建服务器套接字--->设置地址复用--->绑定--->监听

2. 创建epoll实例用来管理套接字

3. 首先将服务器添加到epoll实例上

4. 循环等待事件发生

   ```c++
   while(1){
   	event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
       .....
       for (int i = 0; i < event_count; i++) {
       	if (events[i].data.fd == server_fd) {
               //处理新连接
               accept...
           }else{
               //处理已连接的数据
               int bytes_read = read(client_fd, buffer, BUFFER_SIZE);//将数据读取到buffer中
               if (bytes_read == 0) {
                   //连接断开
               }else if (bytes_read > 0){
                   if (strcmp(message_type, "TEXTMSG ") != 0 && strcmp(message_type, "GETFILE ") != 0){
                       //表示收到的消息是没有包头的文件
                       //在这里直接下载保存到服务器本地
                   }
                   //循环遍历所有连接 读到数据包
                   for (int j = 0; j < client_count+1; j++) {
                       if (client_fds[j] != 0 && client_fds[j] != client_fd && 
                       strcmp(message_type, "TEXTMSG ") == 0) {
                           //处理文本消息  发送给除了发送端之外的所有客户端
                       }else if (client_fds[j] != 0 && client_fds[j] == client_fd && 
                       strcmp(message_type, "GETFILE ")==0){
                           //处理下载请求  发送给请求的客户端
                       }else if (client_fds[j] != 0 && strcmp(message_type, "TEXTMSG ") != 0 
                       && strcmp(message_type, "GETFILE ") != 0){
                           //将文件名发送给所有客户端
                       }
                   }
               }
               
           }
       }
   }
   ```

   

### 效果展示：
![image](https://github.com/davincideng/QChat/blob/main/img/%E5%9B%BE%E7%89%874.png)


## 客户端

### 实现功能：

1. 设计客户端的ui界面，具体展示效果图
2. 创建套接字与服务端进行连接
3. QListWidget显示提示及连接断开信息、QListView显示聊天室内已上传的文件目录。
4. 头像设置为按钮，点击可进行头像选择。编辑框可输入名称。
5. 输入ip、端口号进行连接。

### 效果展示：

- 客户端界面展示
  
![image](https://github.com/davincideng/QChat/blob/main/img/%E5%9B%BE%E7%89%872.png)
- 多客户端间通信
  
![image](https://github.com/davincideng/QChat/blob/main/img/%E5%9B%BE%E7%89%871.png)
- 头像选择（注：目前只能选择提供的头像否则可能无法识别）
  
![image](https://github.com/davincideng/QChat/blob/main/img/%E5%9B%BE%E7%89%873.png)
- 下载文件
  
![image](https://github.com/davincideng/QChat/blob/main/img/%E5%9B%BE%E7%89%875.png)

















