#include "Server.h"
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cassert>
#include"ThreadPool.h"

const int BUFFER_SIZE=1024;

//!Acceptor类，用于监听客户端的连接请求

Acceptor::Acceptor(Epoll* epoll_loop){
    server_sock=new Socket();
    InetAddr* server_inet_addr=new InetAddr("127.0.0.1",3333);
    server_sock->bind(server_inet_addr);
    server_sock->listen();
    server_sock->set_non_blocking();
    //todo
    accept_channel=new Channel(epoll_loop,server_sock->get_sock_fd());
    std::function<void()>read_callback=std::bind(&Acceptor::accept_connection,this);
    accept_channel->set_read_callback(read_callback);
    accept_channel->enable_read();

    delete server_inet_addr;
}

Acceptor::~Acceptor(){
    delete server_sock;
    delete accept_channel;
}

void Acceptor::accept_connection(){
    InetAddr* client_inet_addr=new InetAddr();
    Socket* client_sock=new Socket(server_sock->accept(client_inet_addr));
    printf("New client fd:%d,ip:%s,port:%d\n",client_sock->get_sock_fd(),inet_ntoa(client_inet_addr->get_addr().sin_addr),ntohs(client_inet_addr->get_addr().sin_port));
    client_sock->set_non_blocking();

    new_connection_callback(client_sock);

    delete client_inet_addr;
}

void Acceptor::set_new_connection_callback(std::function<void(Socket*)> new_connection_callback){
    this->new_connection_callback=new_connection_callback;
}

//!Connection类，用于处理客户端的连接请求

Connection::Connection(Epoll* epoll_loop,Socket* client_sock):epoll_loop(epoll_loop),client_sock(client_sock){
    read_buffer=new Buffer();
    write_buffer=new Buffer();
    state=State::CONNECTED;
    if(epoll_loop){
        channel=new Channel(epoll_loop,client_sock->get_sock_fd());
        channel->enable_read();
        channel->set_ET();
    }
}

Connection::~Connection(){
    if(epoll_loop)
        delete channel;
    delete client_sock;
    delete read_buffer;
    delete write_buffer;
}

Connection::State Connection::get_state(){
    return state;
}

void Connection::set_write_buffer(const char* str){
    write_buffer->set_buf(str);
}

Buffer* Connection::get_read_buffer(){
    return read_buffer;
}
Buffer* Connection::get_write_buffer(){
    return write_buffer;
}
const char* Connection::get_read_buffer_as_str(){
    return read_buffer->c_str();
}
const char* Connection::get_write_buffer_as_str(){
    return write_buffer->c_str();
}

Socket* Connection::get_socket(){
    return client_sock;
}

void Connection::deal_with_message(){
    this->read();
    on_message_callback(this);
}

void Connection::read_non_blocking(){
    int sock_fd=client_sock->get_sock_fd();
    char buf[BUFFER_SIZE];
    while(true){
        //read_buffer->clear();
        //ssize_t read_bytes=client_sock->read(read_buffer,BUFFER_SIZE);
        memset(buf,0,BUFFER_SIZE);
        ssize_t read_bytes=::read(sock_fd,buf,sizeof(buf));
        if(read_bytes>0){
            read_buffer->append(buf,read_bytes);
            //printf("READ %s\n",read_buffer->c_str());
        }
        else{
            if(read_bytes==-1&&errno==EINTR){
                printf("continue reading\n");
                continue;
            }
            else if(read_bytes==-1&&((errno==EAGAIN)||(errno==EWOULDBLOCK))){
                break;
            }
            else if(read_bytes==0){
                printf("EOF,client fd %d closed\n",client_sock->get_sock_fd());
                state=State::CLOSED;
                break;
            }
            else{
                printf("Error:Other error happen on client fd %d\n",client_sock->get_sock_fd());
                state=State::CLOSED;
                break;
            }
        }
    }
}

void Connection::write_non_blocking(){
    int client_sock_fd=client_sock->get_sock_fd();
    char buf[write_buffer->get_size()];
    //memcpy(buf,write_buffer->c_str(),write_buffer->get_size());
    for(int i=0;i<write_buffer->get_size();i++){
        buf[i]=write_buffer->c_str()[i];
    }
    buf[write_buffer->get_size()]='\0';
    printf("WRITE %s\n",buf);
    int data_len=write_buffer->get_size();
    int data_left=data_len;
    while(data_left>0){
        ssize_t write_bytes=::write(client_sock_fd,buf+data_len-data_left,data_left);
        if(write_bytes==-1&&errno==EINTR){
            printf("continue writing\n");
            continue;
        }
        else if(write_bytes==-1&&(errno==EAGAIN)){
            break;
        }
        else if(write_bytes==-1){
            printf("Other error happen on client fd %d\n",client_sock_fd);
            state=State::CLOSED;
            break;
        }
        data_left-=write_bytes;
    }
}

// TODO: 
void Connection::read_blocking(){
    //ssize_t read_bytes=client_sock->read(read_buffer,BUFFER_SIZE);
    int sock_fd=client_sock->get_sock_fd();
    unsigned int rcv_size=0;
    socklen_t len=sizeof(rcv_size);
    getsockopt(sock_fd,SOL_SOCKET,SO_RCVBUF,&rcv_size,&len);
    char buf[rcv_size];
    memset(buf,0,sizeof(buf));
    ssize_t read_bytes=::read(sock_fd,buf,sizeof(buf));
    if(read_bytes>0)
        read_buffer->append(buf,read_bytes);
    else if(read_bytes==0){
        printf("EOF,client fd %d closed\n",client_sock->get_sock_fd());
        state=State::CLOSED;
    }
    else if(read_bytes==-1){
        printf("Error:Other error happen on client fd %d\n",client_sock->get_sock_fd());
        state=State::CLOSED;
    }
}

void Connection::write_blocking(){
    int client_sock_fd=client_sock->get_sock_fd();
    ssize_t write_bytes=::write(client_sock_fd,write_buffer->c_str(),write_buffer->get_size());
    if(write_bytes==-1){
        printf("Other error happen on client fd %d\n",client_sock_fd);
        state=State::CLOSED;
    }
}

void Connection::read(){
    assert(state==State::CONNECTED&&"Connection disconnected");
    read_buffer->clear();
    if(client_sock->is_non_blocking()){
        read_non_blocking();
    }
    else{
        read_blocking();
    }
}

void Connection::write(){
    assert(state==State::CONNECTED&&"Connection disconnected");
    if(client_sock->is_non_blocking()){
        write_non_blocking();
    }
    else{
        write_blocking();
    }
    write_buffer->clear();
}

void Connection::close(){
    end_connection_callback(client_sock);
}

void Connection::send(const char* str){
    set_write_buffer(str);
    printf("Going to write %s\n",write_buffer->c_str());
    write();
}

void Connection::set_end_connection_callback(std::function<void(Socket*)> const& cb){
    end_connection_callback=cb;
}
void Connection::set_on_connect_callback(std::function<void(Connection*)> const& cb){
    on_connect_callback=cb;
}
void Connection::set_on_message_callback(std::function<void(Connection*)> const& cb){
    on_message_callback=cb;
    std::function<void()>deal_with_msg=std::bind(&Connection::deal_with_message,this);
    channel->set_read_callback(deal_with_msg);
}


//!Server类，用于管理连接

void Server::set_on_connect_callback(std::function<void(Connection*)> cb){
    on_connect_callback=std::move(cb);
}

void Server::set_on_message_callback(std::function<void(Connection*)> cb){
    on_message_callback=std::move(cb);
}

void Server::set_new_connect_callback(std::function<void(Connection*)> cb){
    new_connect_callback=std::move(cb);
}


void Server::new_connection(Socket* client_sock){
    int client_sock_fd=client_sock->get_sock_fd();
    if(client_sock_fd!=-1){
        int index=client_sock_fd%sub_reactor_loops.size();
        Connection* conn=new Connection(sub_reactor_loops[index],client_sock);
        std::function<void(Socket*)>callback=std::bind(&Server::remove_connection,this,std::placeholders::_1);
        conn->set_end_connection_callback(callback);
        conn->set_on_message_callback(on_message_callback);
        connections[client_sock_fd]=conn;

        if(new_connect_callback){
            new_connect_callback(conn);
        }
    }
}

void Server::remove_connection(Socket* client_sock){
    int client_sock_fd=client_sock->get_sock_fd();
    if(client_sock_fd!=-1){
        auto it=connections.find(client_sock_fd);
        if(it!=connections.end()){
            Connection* conn=connections[client_sock_fd];
            connections.erase(client_sock_fd);
            delete conn;
            conn=nullptr;
        }
    }
}

Server::Server(Epoll* epoll_loop):main_reactor_loop(epoll_loop){
    acceptor=new Acceptor(main_reactor_loop);
    std::function<void(Socket*)>callback=std::bind(&Server::new_connection,this,std::placeholders::_1);
    acceptor->set_new_connection_callback(callback);

    int size=static_cast<int>(std::thread::hardware_concurrency());
    for(int i=0;i<size;i++){
        sub_reactor_loops.push_back(new Epoll());
    }
    thread_pool=new ThreadPool(size);
    for(int i=0;i<size;i++){
        std::function<void()>sub_loop=std::bind(&Epoll::loop,sub_reactor_loops[i]);
        thread_pool->enqueue(std::move(sub_loop));
    }
}

Server::~Server(){
    delete acceptor;
    delete thread_pool;
}