#ifndef _REPOSITORY_H_
#define _REPOSITORY_H_

#include"Commit.h"
#include"Blob.h"
#include"Stage.h"
#include"Tree.h"
#include"Tools.h"

#include<string>
#include<cstring>
#include<filesystem>
#include<unordered_set>

using namespace std;
using std::filesystem::current_path;

//! @brief 仓库类，管理commit、blob、stage、tree等数据结构
class Repository {
public:
    static const string CWD;//获取当前目录
    static const string MYGIT_DIR;//.mygit路径
    static const string OBJECTS_DIR;//存放commit和blob的路径
    static const string REFS_DIR;//
    static const string HEADS_DIR;//存放branch的路径
    static const string HEAD;//当前branch
    static const string STAGE;//暂存区
    static const string TREE;//commit的树
    static const string USER;//用户名

    static void init();//mygit init
    static void add(string file_name);//mygit add [filename]
    static void remove(string file_name);//mygit remove [filename]
    static void commit(string message);//mygit commit [message]
    static void log();//mygit log
    static void global_log();//mygit global-log
    static void find(string message);//mygit find [message]
    static void status();//mygit status
    static void checkout(string file);//mygit checkout -- [filename]
    static void checkout(string commit_id,string file);//mygit checkout [commit_id] -- [filename]
    static void checkout_commit(string commit_id);//mygit checkout [commit_id]
    static void checkout_branch(string branch_name);//mygit checkout [branch_name]
    static void print_branches();;//mygit branch
    static void create_branch(string branch_name);//mygit branch [branch_name]
    static void remove_branch(string branch_name);//mygit rm-branch [branch_name]
    static void reset(string commit_id);//mygit reset [commit_id]
    static void merge(string branch_name);//mygit merge [branch_name]
    static void push();//mygit push
    static void clone(string path);//mygit clone
    static void set_user(string username);//mygit set [username]
    static void print_tree();//mygit tree
    static void help();//mygit --help
    static void check_if_init();
    static Commit get_cur_commit();
private:
    static void set_cur_branch(string branch_name);
    static void init_commit();
    static void add_commit(Commit commit);
    static Commit get_head_commit_of_branch(string branch_name);
    static Commit get_commit_by_id(string id);
    static Blob get_blob_by_id(string id);
    static void print_commit(Commit commit);
    static Stage read_stage();
    static void print_stage_files();
    static void print_removed_files();
    static void print_modifications_not_staged_for_commit();
    static void print_untracked();
    static void print_file_list(vector<string>file_list);
    static void change_commit_to(string commit_id);
    static Commit get_common_commit(Commit a, Commit b);
    static unordered_set<string>get_all_files(unordered_map<string, string> a, unordered_map<string, string> b, unordered_map<string, string>);
    static string deal_with_conflict(string FILE, string curBlobId, string givBlobId);
    static string get_user_name();
};

/*
.mygit
    |--objects
    |   |--commit and blob
    |--refs
    |   |--heads
    |       |--main
    |--HEAD
    |--stage
    |--USER
    |--TREE
*/


//! @brief 单个操作的抽象类
class SingleOperation{
public:
    int argc;
    char** argv;
    SingleOperation(int argc,char**argv);
    virtual void execute()=0;
};
//! @brief mygit init命令的执行对象
class InitOperation:public SingleOperation{
public:
    InitOperation(int argc,char**argv);
    void execute();
};


//! @brief mygit add命令的执行对象
class AddOperation:public SingleOperation{
public:
    AddOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit remove命令的执行对象
class RemoveOperation:public SingleOperation{
public:
    RemoveOperation(int argc,char**argv);
    void execute();
};
//! @brief mygit commit命令的执行对象
class CommitOperation:public SingleOperation{
public:
    CommitOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit log命令的执行对象
class LogOperation:public SingleOperation{
public:
    LogOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit global-log命令的执行对象
class GlobalLogOperation:public SingleOperation{
public:
    GlobalLogOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit find命令的执行对象
class FindOperation:public SingleOperation{
public:
    FindOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit status命令的执行对象
class StatusOperation:public SingleOperation{
public:
    StatusOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit checkout [commitid] -- [filename]命令的执行对象
class CheckoutCommitAndFileOperation:public SingleOperation{
public:
    CheckoutCommitAndFileOperation(int argc,char**argv);
    void execute();
};
//! @brief mygit checkout [commitid]命令的执行对象
class CheckoutCommitOperation:public SingleOperation{
public:
    CheckoutCommitOperation(int argc,char**argv);
    void execute();
};
//! @brief mygit checkout -- [filename]命令的执行对象
class CheckoutFileOperation:public SingleOperation{
public:
    CheckoutFileOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit checkout [branch_name]命令的执行对象
class CheckoutBranchOperation:public SingleOperation{
public:
    CheckoutBranchOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit branch命令的执行对象
class PrintBranchesOperation:public SingleOperation{
public:
    PrintBranchesOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit branch [branch_name]命令的执行对象
class CreateBranchOperation:public SingleOperation{
public:
    CreateBranchOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit rm-branch [branch_name]命令的执行对象
class RemoveBranchOperation:public SingleOperation{
public:
    RemoveBranchOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit reset [commit_id]命令的执行对象
class ResetOperation:public SingleOperation{
public:
    ResetOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit merge [branch_name]命令的执行对象
class MergeOperation:public SingleOperation{
public:
    MergeOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit push命令的执行对象
class PushOperation:public SingleOperation{
public:
    PushOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit clone命令的执行对象
class CloneOperation:public SingleOperation{
public:
    CloneOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit set [username]命令的执行对象
class SetUserOperation:public SingleOperation{
public:
    SetUserOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit tree命令的执行对象
class PrintTreeOperation:public SingleOperation{
public:
    PrintTreeOperation(int argc,char**argv);
    void execute();
};

//! @brief mygit --help命令的执行对象
class HelpOperation:public SingleOperation{
public:
    HelpOperation(int argc,char**argv);
    void execute();
};
//! @brief 空操作的执行对象
class NullOperation:public SingleOperation{
public:
    NullOperation(int argc,char**argv);
    void execute();
};
//! @brief 无效操作的执行对象
class IncorrectOperands:public SingleOperation{
public:
    IncorrectOperands(int argc,char**argv);
    void execute();
};
//! @brief 工厂类，根据命令创建对应的执行对象
class Factory{
public:
    static SingleOperation* create(string op,int argc,char**argv);
};

#endif