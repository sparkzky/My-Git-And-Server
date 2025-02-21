#include "AidClass.h"
#include "Client.h"
#include "../../MyGit/include/Tools.h"
#include "../../MyGit/include/Repository.h"
#include<cstring>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <fstream>

const int BUFFER_SIZE=1024;

Client::Client(){
    socket=new Socket();
    InetAddr* server_inet_addr=new InetAddr("127.0.0.1",3333);
    socket->connect(server_inet_addr);

    send_buffer=new Buffer();
    recv_buffer=new Buffer();
    delete server_inet_addr;
}

Client::~Client(){
    delete socket;
    delete send_buffer;
    delete recv_buffer;
}

void Client::write_message(string msg){
    if(msg!=""){
        send_buffer->set_buf(msg.c_str());
        //printf("Going to write message: %s\n",msg.c_str());
    }

    //ssize_t write_bytes=socket->write(send_buffer);
    ssize_t write_bytes=write(socket->get_sock_fd(),send_buffer->c_str(),send_buffer->get_size());

    if(write_bytes==-1){
        printf("Socket disconnected,can't send message!\n");
        return;
    }
}

void Client::send(string str){
    write_message(str);
    char buf[BUFFER_SIZE];

    while(true){
        //recv_buffer->clear();
        //ssize_t read_bytes=socket->read(recv_buffer,BUFFER_SIZE);
        bzero(&buf, sizeof(buf));
        ssize_t read_bytes=read(socket->get_sock_fd(),buf,sizeof(buf));
        if(read_bytes>0){
            //string content=recv_buffer->get_content_as_string();
            string content(buf);
            if(content.substr(0,8)=="received"){
                if(content.size()>8){
                    printf("Server received message: %s\n",content.substr(9).c_str());
                }
                break;
            }
        }
        else if(read_bytes==0){
            printf("Server disconnected,can't receive message!\n");
            exit(EXIT_SUCCESS);
        }
    }
}

void Client::send_file(string filename){
    send("file:"+filename);
    ifstream fin(filename);
    while(true){
        send_buffer->getline(fin);
        if(send_buffer->get_size()==0){
            send(":end");
            break;
        }
        send();
    }
}
//clone
void Client::recv(string path){
    int sock_fd=socket->get_sock_fd();
    this->write_message("clone:"+path);
    string project=get_filename(path);
    string project_path=join(Repository::CWD,project);
    char buf[BUFFER_SIZE];
    while(true){
        //recv_buffer->clear();//?怎么只能读八个字节
        //ssize_t read_bytes=socket->read(recv_buffer,BUFFER_SIZE);
        bzero(&buf,sizeof(buf));
        ssize_t read_bytes=::read(sock_fd,buf,sizeof(buf));
        if(read_bytes==0){
            printf("Server disconnected,can't receive message!\n");
            exit(EXIT_SUCCESS);
        }
        //string content=recv_buffer->get_content_as_string();
        string content(buf);
        if(content.substr(0,5)=="file:"){
            const string filename=content.substr(5);
            const string save_path=join(project_path,filename);
            mkdir_by_path(save_path);

            fout.open(save_path);
            write_message("received");
        }
        else if(content==":end"){
            fout.close();
            write_message("received");
        }
        else if(content==":allend"){
            break;
        }
        else{
            fout<<content<<endl;

            write_message("received");
        }
    }
}