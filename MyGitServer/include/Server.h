#ifndef _SERVER_H_
#define _SERVER_H_
#include"AidClass.h"
#include"EpollEvent.h"
#include"ThreadPool.h"
#include"Tool.h"
#include<functional>
#include<iostream>
#include<string>
#include<fstream>
#include<map>
#include<vector>
//acceptor通常指的是一个已经通过listen()方法设置为监听状态的套接字。当有客户端尝试连接到服务器时，acceptor会阻塞在accept()方法上，直到有新的连接请求到达。
//一旦acceptor接受了客户端的连接请求，它就会创建一个新的套接字，这个套接字代表了与客户端的connection。这个新的套接字可以用来与客户端进行双向通信。

class Acceptor;
class Connection;
class Server;

//! @brief 代表一个监听套接字
// 负责监听客户端的连接请求，并创建新的Connection对象来代表与客户端的连接。
class Acceptor {
    Socket* server_sock;//监听的socket
    Epoll* epoll_loop;//epoll事件循环
    Channel* accept_channel;//用于监听套接字的事件
    std::function<void(Socket*)> new_connection_callback;//新连接到来时的个人设置的回调函数，会在accept_connection()中被调用
public:
    Acceptor(Epoll* epoll_loop);
    void set_new_connection_callback(std::function<void(Socket*)> callback);//设置新连接到来时的个人设置的回调函数
    ~Acceptor();
    void accept_connection();//接受新的连接请求后的回调函数
};


//! @brief 代表一个与客户端的连接
//  这个类代表了一个与客户端的连接，它包含了与客户端的通信相关的功能，包括读写数据、发送数据等。
class Connection {
public:
    enum State{
        INVALID=1,
        HANDSHAKING,
        CONNECTED,
        CLOSED,
        FAILED,
    };//定义当前connection状态的枚举类型
private:
    Epoll* epoll_loop;//epoll事件循环
    Socket* client_sock;//accept之后返回的与客户端连接的socket
    Channel* channel;//当前connection的事件
    State state;//当前connection的状态
    Buffer* read_buffer;//读缓冲区
    Buffer* write_buffer;//写缓冲区

    std::function<void(Socket*)>end_connection_callback;//连接关闭时的回调函数
    std::function<void(Connection*)>on_connect_callback;//连接成功时的回调函数,没用到
    std::function<void(Connection*)>on_message_callback;//接收到消息时的回调函数

    void read_non_blocking();//非阻塞读
    void write_non_blocking();//非阻塞写
    void read_blocking();//阻塞读
    void write_blocking();//阻塞写

public:
    ostream& fout=std::cout;
    ofstream file;//client push的时候用于写入文件
    string username="";//connection的用户名
    string project="";//connection的项目名

    Connection(Epoll* epoll_loop,Socket* client_sock);
    ~Connection();

    void read();//读数据
    void write();//写数据
    void send(const char *);//发送数据
    void close();//关闭连接

    void set_end_connection_callback(std::function<void(Socket*)> const& cb);//设置连接关闭时的回调函数
    void set_on_connect_callback(std::function<void(Connection*)> const& cb);//设置连接成功时的回调函数，没用到
    void set_on_message_callback(std::function<void(Connection*)> const& cb);//设置接收到消息时的回调函数

    State get_state();//获取当前connection的状态

    void set_write_buffer(const char* str);//设置写缓冲区
    Buffer* get_read_buffer();//获取读缓冲区
    Buffer* get_write_buffer();//获取写缓冲区
    const char* get_read_buffer_as_str();//获取读缓冲区的字符串形式
    const char* get_write_buffer_as_str();//获取写缓冲区的字符串形式

    Socket* get_socket();//获取当前connection的client_socket

    void deal_with_message();//根据接收到的消息处理事件
};
//! @brief 服务器类，负责监听客户端的连接请求，并创建新的Connection对象来代表与客户端的连接。
class Server{
    Epoll* main_reactor_loop;//主事件循环
    std::vector<Epoll*> sub_reactor_loops;//子事件循环
    Acceptor* acceptor;//监听套接字
    std::map<int,Connection*> connections;//所有连接的集合
    ThreadPool* thread_pool;//线程池

    std::function<void(Connection*)> on_connect_callback;//连接成功时的具体回调函数，会传递给connection对象，没用到
    std::function<void(Connection*)> on_message_callback;//接收到消息时的具体回调函数，会传递给connection对象
    std::function<void(Connection*)> new_connect_callback;//新连接到来时的具体回调函数，会传递给connection对象

public:
    Server(Epoll* main_epoll_loop);
    ~Server();

    void new_connection(Socket* client_sock);//新连接到来时的回调函数
    void remove_connection(Socket* client_sock);//移除连接时的回调函数
    void set_on_connect_callback(std::function<void(Connection*)>  cb);//设置连接成功时的具体回调函数
    void set_on_message_callback(std::function<void(Connection*)>  cb);//设置接收到消息时的具体回调函数
    void set_new_connect_callback(std::function<void(Connection*)>  cb);//设置新连接到来时的具体回调函数



};

/*

在Server类定义的时候会设置具体的回调函数
new_connection_callback是传给Acceptor，使其监听到新连接后生成Connection类对象并设置read_callback函数和remove_connection_callback函数
Connection接收到客户端发送来的信息后调用read_callback处理
Acceptor和Connection底层都是靠epoll的监听的channel事件，都会被加入Epoll中监听
channel被设置为ready后会被poll下来，在loop函数里面被调用
Epoll靠线程池来loop，进而执行每一个channel事件

*/

#endif