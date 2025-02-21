#ifndef FACTORY_H
#define FACTORY_H
#include"Server.h"
#include"../../MyGit/include/Tools.h"

class Operation{
    Connection* conn;
    string msg;
public:
    Operation(Connection* conn,const std::string msg);
    virtual void execute()=0;
    Connection* get_conn();
    string get_msg();
};

class Operation_for_username:public Operation{
    string cur_path;
public:
    Operation_for_username(Connection* conn,const std::string& msg,string cur_path);
    void execute();
};

class Operation_for_project:public Operation{
    string cur_path;
public:
    Operation_for_project(Connection* conn,const std::string& msg,string cur_path);
    void execute();
};

class Operation_for_clone:public Operation{
    string cur_path;
public:     
    Operation_for_clone(Connection* conn,const std::string& msg,string cur_path);
    void execute();
};

class Operation_for_file:public Operation{
    string cur_path;
public:
    Operation_for_file(Connection* conn,const std::string& msg,string cur_path);
    void execute();
};

class Operation_for_end:public Operation{
public:
    Operation_for_end(Connection* conn);
    void execute();
};

class Operation_for_write:public Operation{
public:
    Operation_for_write(Connection* conn,const std::string& msg);
    void execute();
};

class OperationFactory{
public:
    static Operation* create_operation(Connection* conn,std::string op_type,string cur_path);
};

#endif