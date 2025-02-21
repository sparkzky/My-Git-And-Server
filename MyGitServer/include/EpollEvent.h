#ifndef _EPOLLEVENT_H_
#define _EPOLLEVENT_H_

#include <sys/epoll.h>
#include<functional>
#include<vector>
using namespace std;

class Epoll;
class Channel;
class EventLoop;


//在使用 epoll 时，应用程序会创建一个 epoll 实例，
//并使用 epoll_ctl 系统调用将感兴趣的事件和文件描述符添加到 epoll 实例中。
//当事件发生时，epoll_wait 系统调用会返回一个 epoll_event 结构体的数组，其中包含了就绪的文件描述符和相应的事件类型。

//! @brief 封装了epoll的相关操作的类
class Epoll{
    int epoll_fd;//epoll句柄
    epoll_event* epoll_events;//epoll事件数组
    bool quit;//终止标志
public:
    Epoll();
    ~Epoll();

    void update_channel(Channel* channel);//包含了注册、修改事件的操作
    void remove_channel(Channel* channel);//删除事件的操作
    vector<Channel*> poll(int timeout=-1);//等待事件发生并返回就绪的Channel列表
    void loop();//事件循环
    void quit_loop();//终止事件循环
};

// typedef union epoll_data {
//     void        *ptr;
//     int          fd;
//     uint32_t     u32;
//     uint64_t     u64;
// } epoll_data_t;

// struct epoll_event {
//     uint32_t     events;      /* Epoll events */
//     epoll_data_t data;        /* User data variable */
// };
//! @brief 封装了epoll事件的相关操作的类,channel代表一个抽象的I/O操作
class Channel{
    Epoll* epoll_loop;//所属的epoll实例
    int channel_fd;//文件描述符
    uint32_t events;//事件类型
    bool in_epoll;//是否启用
    function<void()> read_callback;//读事件回调函数
    function<void()> write_callback;//写事件回调函数
    function<void()> error_callback;//错误事件回调函数，没用到
public:
    Channel(Epoll* loop, int channel_fd);
    ~Channel();

    void set_read_callback(function<void()> callback);//设置读事件回调函数
    void set_in_epoll(bool in_epoll=true);//设置是否已经加入epoll实例中
    void set_events(uint32_t);//设置事件类型
    void set_write_callback(function<void()> callback);//设置写事件回调函数
    void set_ET();//设置边缘触发模式

    void enable_read();//设置为读事件并加入epoll实例中
    void disable_read();//设置为非读事件并从epoll实例中更新事件类型
    void handle_event();//处理事件，调用相应的回调函数

    int get_channel_fd();//获取文件描述符
    uint32_t get_events();//获取事件类型
    bool is_in_epoll();//判断是否已经加入epoll实例中
};
#endif


/*Channel（通道）
Channel是网络编程中的一个抽象概念，它通常代表一个打开的文件描述符（file descriptor），例如一个网络连接。在非阻塞IO中，一个Channel可以配置为非阻塞模式，这样在执行IO操作时，如果操作不能立即完成，它会返回一个错误而不是阻塞。

Epoll（事件通知机制）
epoll是Linux特有的I/O多路复用机制。它允许程序监控多个文件描述符的读写事件，并且当这些事件就绪时接收到通知。epoll比传统的select和poll机制更为高效，因为它使用了内核级别的数据结构来管理文件描述符，减少了应用程序和内核之间的交互次数。

Event Loop（事件循环）
Event Loop是网络服务器或客户端的心脏，它负责监控Channel上的事件，并调用相应的事件处理器。在事件循环中，通常会使用epoll来等待事件的发生。当一个Channel准备好进行IO操作时，Event Loop会触发相应的事件处理器来进行处理。*/