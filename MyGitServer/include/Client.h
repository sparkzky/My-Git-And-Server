#ifndef _CLIENT_H_
#define _CLIENT_H_

#include"AidClass.h"
#include<iostream>
#include<string>
#include<fstream>

using std::istream;
using std::ofstream;
using std::string;

//! @brief Client类，用于与服务端通信
//! @details 该类用于与服务端通信，包括发送消息、接收消息、发送文件等功能。
class Client{
    Socket* socket;// Socket类指针，用于与服务端通信
    Buffer* recv_buffer;// 接收消息的缓冲区
    Buffer* send_buffer;// 发送消息的缓冲区
    
    void write_message(string msg="");//发送消息的基本实现
public:
    Client();
    ~Client();
    void send(string str="");//发送消息
    void recv(string path);//接收路径为path的文件
    void send_file(string filename);//发送文件

    ofstream fout;//用于clone时在本地写入文件
};



#endif