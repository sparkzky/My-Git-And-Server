#include "Factory.h"
#include<unistd.h>

Operation* OperationFactory::create_operation(Connection* conn,std::string op_type,string cur_path){
    if(op_type.substr(0,9)=="username:"){
        return new Operation_for_username(conn,op_type.substr(9),cur_path);
    }

    if(op_type.substr(0,8)=="project:"){
        return new Operation_for_project(conn,op_type.substr(8),cur_path);
    }

    if(op_type.substr(0,6)=="clone:"){
        return new Operation_for_clone(conn,op_type.substr(6),cur_path);
    }

    if(op_type.substr(0,5)=="file:"){
        return new Operation_for_file(conn,op_type.substr(5),cur_path);
    }
    else if(op_type.substr(0,4)==":end"){
        return new Operation_for_end(conn);
    }
    else{
        return new Operation_for_write(conn,op_type);
    }
}

Operation::Operation(Connection* conn,const std::string msg):conn(conn),msg(msg){}

Connection* Operation::get_conn(){
    return this->conn;
}

string Operation::get_msg(){
    return this->msg;
}

Operation_for_username::Operation_for_username(Connection* conn,const std::string& msg,string cur_path)
    :Operation(conn,msg),cur_path(cur_path){}

void Operation_for_username::execute(){
    Connection* connection=this->get_conn();
    connection->username=this->get_msg();
    string dir=join(cur_path,connection->username);
    if(access(dir.c_str(),0)==-1){
        mkdir(dir.c_str(),S_IRWXU);
    }
    printf("new user %s\n",connection->username.c_str());
    connection->send("received");
    return;
}

Operation_for_project::Operation_for_project(Connection* conn,const std::string& msg,string cur_path)
    :Operation(conn,msg),cur_path(cur_path){}

void Operation_for_project::execute(){
    Connection* conn=this->get_conn();
    conn->project=this->get_msg();
        if(conn->username==""){
            Exit("User not exist!");
        }
        string dir_name=join(cur_path,join(conn->username,conn->project));
        if(access(dir_name.c_str(),0)==-1){
            mkdir(dir_name.c_str(),S_IRWXU);
        }
        conn->send("received");
        return;
}

Operation_for_clone::Operation_for_clone(Connection* conn,const std::string& msg,string cur_path)
    :Operation(conn,msg),cur_path(cur_path){}

void Operation_for_clone::execute(){
    Connection* conn=this->get_conn();
    string dir_path=this->get_msg();
    dir_path=join(cur_path,dir_path);

    printf("client trying to clone %s\n",dir_path.c_str());
    queue<string>files_queue;
    files_queue.push(dir_path);
    while(!files_queue.empty()){
        string file=files_queue.front();
        files_queue.pop();
        if(is_dir(file)){
            vector<string>files=plain_file_names_in(file);
            for(auto it:files){
                files_queue.push(join(file,it));
            }
        }
        else{
            ifstream fin(file);
            string line="file:"+get_relative_path(dir_path,file);
            while(true){
                if(line=="")getline(fin,line);
                if(line.size()==0)line=":end";
                conn->send(line.c_str());
                while(true){
                    if(conn->get_state()==Connection::State::CLOSED){
                        conn->close();
                        return;
                    }
                    conn->read();
                    string recv_msg(conn->get_read_buffer_as_str());
                    if(recv_msg=="received"){
                        break;
                    }
                }
                if(line==":end"){
                    break;
                }
                line="";
            }
        }
    }

    conn->send(":allend");
    printf("client fd %d disconnected\n",conn->get_socket()->get_sock_fd());
    conn->close();
    return;
}

Operation_for_file::Operation_for_file(Connection* conn,const std::string& msg,string cur_path)
    :Operation(conn,msg),cur_path(cur_path){}

void Operation_for_file::execute(){
    Connection* conn=this->get_conn();
    const string filename=this->get_msg();
    string save_path=join(join(join(cur_path,conn->username),conn->project),filename);
    printf("file save path: %s\n", save_path.c_str());
    mkdir_by_path(save_path);

    conn->file.open(save_path);
    conn->fout.rdbuf(conn->file.rdbuf());
    conn->send("received");
}

Operation_for_end::Operation_for_end(Connection* conn):Operation(conn,""){}

void Operation_for_end::execute(){
    Connection* conn=this->get_conn();
    conn->file.close();
    string msg="received file\n";
    conn->send(msg.c_str());
}

Operation_for_write::Operation_for_write(Connection* conn,const std::string& msg):Operation(conn,msg){}

void Operation_for_write::execute(){
    Connection* conn=this->get_conn();
    string msg=this->get_msg();
    conn->fout<<msg.c_str()<<endl;
    conn->send("received");
}