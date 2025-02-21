#ifndef _COMMIT_H_
#define _COMMIT_H_

#include<string>
#include<vector>
#include<unordered_map>
#include<time.h>
using namespace std;

//! @brief 表示每一个提交(commit)信息的数据格式
class Commit {
	string message;//提交信息
	string id;//标识符
	vector<string>parents;//父节点
	unordered_map<string, string>files_to_blobs;//blob文件地址与对应的id
	string date;//日期
	string commit_file;//存储commit相关信息
public:
	Commit();
	Commit(string message, vector<string> parents, unordered_map<string, string> blobs);

	string get_id();//获取标识符
	unordered_map<string,string> get_files_to_blobs();//获取文件地址与对应的blob_id
	vector<string>get_parents();//获取父节点
	string get_message();//获取提交信息
	string get_date();//获取日期
	
	void save();//保存更改，并写入保存的文件
	static Commit read_commitfile_as_commit(string commit_file);//将地址的文件以commit数据格式返回

private:
	string generate_id();//得到独特的id
	string generate_commit_file_route();//生成文件的地址（id对应的
	string get_time_to_set();//得到当前时间
};


#endif