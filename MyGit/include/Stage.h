#ifndef _STAGE_H_
#define _STAGE_H_

#include"Blob.h"

#include<unordered_map>
#include<unordered_set>
#include<string>

using namespace std;


//! @brief 工作区类，用于记录新增和删除的文件
class Stage {
	unordered_map<string, string>added_object;//增加的更改的记录
	unordered_set<string>removed_object;//去除的更改的记录

public:
	unordered_map<string, string>get_added_object();//获取新增记录
	unordered_set<string>get_removed_object();//获取去除记录

	void save();//保存工作区到文件
	bool add(string file);//返回是否增加成功
	bool remove(string file);//返回是否减少成功

	bool is_empty();//判断工作区是否为空
	void clear();//清空工作区
	static Stage read_stagefile_as_stage(string stage_file);//将该路径的文件以StageArea数据格式返回
private:
	bool is_new_blob(Blob blob, string path);//判断要增加的是不是新的blob
};

#endif