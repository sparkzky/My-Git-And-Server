#ifndef _BLOB_H_
#define _BLOB_H_

#include<iostream>
#include<vector>
using namespace std;

//! @brief 表示每一个源文件的信息的数据格式
class Blob{
	string id;//唯一标识
	string src_file;//对应的源文件
	string blob_file;//在自定义的文件夹中的路径
	vector<string>content;//具体内容

public:
	Blob(string src_file);//用源文件初始化
	Blob(string id, string src_file, string blob_file, vector<string> content);//全部初始化
	
	string get_id();//获取该blob文件的id标识
	string get_content_as_string();//将源文件的内容（content）获取，并以string格式返回
	
	void save();//保存更改到源文件和blob文件
	void save_to_src();//将目前的更改写入源文件
	void save_to_blob();//将目前的更改写入blob文件
	static Blob read_blobfile_as_blob(string blob_file);//将blob文件以blob的形式返回
};

#endif