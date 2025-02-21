#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <sys/stat.h>

using namespace std;

vector<string>read_contents(string file);//获取文件内容

void write_contents(string file, string line);//将line写入文件file

void write_contents(string file, vector<string> contents);//将contents写入文件file

string join(string dir, string name);//拼接路径

string get_parent_file(string file);//获取父目录

void Exit(string message);//退出进程

string to_string(vector<string> contents);//将vector<string>转化为string

string to_string(unordered_map<string,string> umaps);//将unordered_map<string,string>转化为string

string read_contents_as_string(string file);//获取文件内容并转化为string

vector<string>plain_file_names_in(string dir); //获取目录下的文件名

bool starts_with(const string& str, const string& head);//判断str是否以head开头

string get_filename(string file);//获取文件名

string get_relative_path(string dir,string path);// 获取文件在 dir 下的相对位置


void mkdir_by_path(string path);//按照当前路径创建目录或文件

bool is_dir(string path);//判断是否为目录

void validate_args_num(int argc, int n) ;//验证参数个数是否为n

#endif 
