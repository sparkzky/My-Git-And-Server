#include"Server.h"
#include"../../MyGit/include/Tools.h"
#include"Factory.h"
#include<iostream>
#include<string>
#include<fstream>
#include<unistd.h>
#include<queue>
#include<cstdio>

using std::string;
using std::endl;
using std::cout;
using std::ofstream;
using std::queue;
using std::ifstream;

void handle_request(Connection* conn);

int main(){
    Epoll* epoll_loop=new Epoll();
    Server* server=new Server(epoll_loop);

    server->set_new_connect_callback([](Connection* conn){});

    server->set_on_message_callback(handle_request);

    epoll_loop->loop();

    delete server;
    delete epoll_loop;


    return 0;
}

void handle_request(Connection* conn){
    const string CWD="../resource";
    mkdir(CWD.c_str(),S_IRWXU);

    if(conn->get_state()==Connection::State::CLOSED){
        conn->fout.rdbuf((std::cout).rdbuf());
        conn->close();
        return;
    }

    string msg(conn->get_read_buffer_as_str());
    printf("the msg is %s\n",msg.c_str());

    Operation* op=OperationFactory::create_operation(conn,msg,CWD);
    op->execute();

    delete op;

}

