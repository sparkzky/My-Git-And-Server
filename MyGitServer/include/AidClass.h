#ifndef _AIDCLASS_H_
#define _AIDCLASS_H_

#include<arpa/inet.h>
#include <string>
#include<iostream>
#include<fstream>
using namespace std;

//! @brief 封装sockaddr_in和socklen_t的用来表示IP地址和端口的类
class InetAddr {
public:
    sockaddr_in addr;//sockaddr_in结构体
    socklen_t addr_len;//socklen_t长度

    InetAddr();
    InetAddr(const char*ip,uint16_t port);
    ~InetAddr();

    void set_inet_addr(sockaddr_in addr,socklen_t addr_len);//设置addr和addr_len
    sockaddr_in get_addr();//获取addr
    socklen_t get_addr_len();//获取addr_len
};
//! @brief 封装缓冲区的类
class Buffer{
    string buffer;
public:
    Buffer();
    ~Buffer();

    void clear();//清空缓冲区

    void append(const char* str,int size);//追加字符串到缓冲区
    ssize_t get_size();//获取缓冲区大小
    const char* c_str();//将缓冲区以const char *的形式返回
    void set_buf(const char* str);//设置缓冲区内容
    void getline(istream& fin=cin);//从输入流中读取一行到缓冲区
    string get_content_as_string();//获取缓冲区内容
};
//! @brief 封装socket的类，实现socket的基本操作
class Socket{
    int sock_fd;//socket的文件描述符
public:
    Socket();
    Socket(int sock_fd);
    ~Socket();

    int get_sock_fd()const;//获取socket的文件描述符

    void bind(InetAddr*);//用于Server的socket绑定到一个地址
    void connect(const char* ip, uint16_t port);//用于Client的socket连接到一个地址
    void connect(InetAddr*);
    void listen();//用于Server的socket开始监听
    
    int accept(InetAddr*);//Server的socket接受链接请求，返回新的Client的socket文件描述符,用来send和recv

    void set_non_blocking();//设置socket为非阻塞模式
    bool is_non_blocking();//判断socket是否为非阻塞模式

    ssize_t read(Buffer* , int size);//从socket读取数据到缓冲区
    ssize_t write(Buffer* );//从缓冲区写入数据到socket
};


#endif