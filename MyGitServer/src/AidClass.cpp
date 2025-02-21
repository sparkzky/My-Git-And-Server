#include "AidClass.h"
#include"Tool.h"
#include<cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

InetAddr::InetAddr():addr_len(sizeof(addr)){
    bzero(&addr,sizeof(addr));
}

InetAddr::InetAddr(const char* ip,uint16_t port):addr_len(sizeof(addr)){
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=inet_addr(ip);
}

InetAddr::~InetAddr(){}

void InetAddr::set_inet_addr(sockaddr_in addr,socklen_t addr_len){
    this->addr=addr;
    this->addr_len=addr_len;
}

sockaddr_in InetAddr::get_addr(){
    return addr;
}

socklen_t InetAddr::get_addr_len(){
    return addr_len;
}

Buffer::Buffer(){}

Buffer::~Buffer(){}

void Buffer::clear(){
    buffer.clear();
}

string Buffer::get_content_as_string(){
    return buffer;
}

void Buffer::append(const char* data,int len){
    for(int i=0;i<len;i++){
        if(data[i]=='\0'){
            break;
        }
        buffer.push_back(data[i]);
    }
}

ssize_t Buffer::get_size(){
    return buffer.size();
}

const char* Buffer::c_str(){
    return buffer.c_str();
}

void Buffer::getline(istream& fin){
    buffer.clear();
    std::getline(fin,buffer);
}

void Buffer::set_buf(const char* str){
    buffer.clear();
    buffer.append(str);
}

Socket::Socket(){
    sock_fd=socket(AF_INET,SOCK_STREAM,0);
    err_if(sock_fd==-1,"create socket failed");
}



Socket::Socket(int sock_fd):sock_fd(sock_fd){
    err_if(sock_fd==-1,"create socket failed");
}

Socket::~Socket(){
    if(sock_fd!=-1){
        close(sock_fd);
        sock_fd=-1;
    }
}

void Socket::bind(InetAddr* server_inet_addr){
    sockaddr_in server_addr=server_inet_addr->get_addr();
    socklen_t server_addr_len=server_inet_addr->get_addr_len();
    err_if(::bind(sock_fd,(sockaddr*)&server_addr,server_addr_len)==-1,"bind socket failed");
}

void Socket::listen(){
    err_if(::listen(sock_fd,SOMAXCONN)==-1,"listen socket failed");
}

int Socket::accept(InetAddr* client_inet_addr){
    int client_sock_fd=::accept(sock_fd,(sockaddr*)&client_inet_addr->addr,&client_inet_addr->addr_len);
    err_if(client_sock_fd==-1,"accept socket failed");
    return client_sock_fd;
}

void Socket::connect(InetAddr* server_inet_addr){
    sockaddr_in server_addr=server_inet_addr->get_addr();
    socklen_t server_addr_len=server_inet_addr->get_addr_len();
    err_if(::connect(sock_fd,(sockaddr*)&server_addr,server_addr_len)==-1,"connect socket failed");
}

void Socket::set_non_blocking(){
    fcntl(sock_fd,F_SETFL,fcntl(sock_fd,F_GETFL,0)|O_NONBLOCK);
}

bool Socket::is_non_blocking(){
    return (fcntl(sock_fd,F_GETFL)&O_NONBLOCK)!=0;
}

ssize_t Socket::read(Buffer* data,int len){
    char* buffer=new char[len];
    err_if(buffer==nullptr,"malloc buffer failed");
    memset(buffer,0,sizeof(buffer));

    ssize_t received_bytes=::read(this->sock_fd,buffer,sizeof(buffer));

    if(received_bytes>0)
        data->append(buffer,received_bytes);
    delete[] buffer;
    return received_bytes;
}

ssize_t Socket::write(Buffer* data){
    return ::write(this->sock_fd,data->c_str(),data->get_size());
}

void Socket::connect(const char* ip, uint16_t port){
    InetAddr* inet_addr=new InetAddr(ip,port);
    connect(inet_addr);
    delete inet_addr;
}

int Socket::get_sock_fd()const{
    return sock_fd;
}