#include "../include/Tools.h"

#include <iostream>
#include <fstream>
#include <numeric>
#include <dirent.h>
#include <sys/types.h>
#include <cstring>
#include <unistd.h>

using namespace std; 

vector<string>read_contents(string file) {
	vector<string>content;
	ifstream fin(file);
	string line;
	while (getline(fin, line)) {
		content.push_back(line);
	}
	fin.close();
	return content;
}

void write_contents(string file, string line) {
	ofstream fout(file);
	fout << line;
	fout.close();
}

void write_contents(string file, vector<string> content) {
	ofstream fout(file);
	for (string line : content) {
		fout << line + '\n';
	}
	fout.close();
}

string join(string dir, string file) {
	return dir + '/' + file;
}

string get_parent_file(string file) {
	for (int i = file.size() - 1; i >= 0; i--) {
		if (file[i] == '/') {
			return file.substr(0, i);
		}
	}
	return "";
}

void Exit(string msg) {
	cerr << msg << endl;
	exit(1);
}

string to_string(vector<string> lines) {
	string text = "";
	return accumulate(lines.begin(), lines.end(), text);
}

string to_string(unordered_map<string, string>umap) {
	string text = "";
	for (auto it : umap) {
		text += it.first + it.second;
	}
	return text;
}

string read_contents_as_string(string file) {
	return to_string(read_contents(file));
}

vector<string>plain_file_names_in(string dir) {
	vector<string>files;
	DIR* pDir;
	struct dirent* ptr;
	if (!(pDir = opendir(dir.c_str()))) {
		cout << "Folder doesn't exist!" << endl;
		exit(1);
	}
	while (ptr = readdir(pDir)) {
		if (strcmp(ptr->d_name, ".") && strcmp(ptr->d_name, "..")) {
			files.push_back(ptr->d_name);
		}
	}
	closedir(pDir);
	return files;
}

bool is_dir(string dir){
	struct stat buffer;
    return (stat (dir.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

bool starts_with(const string& str, const string& head) {
	return str.compare(0, head.size(), head) == 0;
}

string get_filename(string file) {
	string name = "";
	for (int i = file.size() - 1; i >= 0; i--) {
		if (file[i] == '/') {
			break;
		}
		name = file[i] + name;
	}
	return name;
}

string get_relative_path(string dir, string path) {// 获取文件在 dir 下的相对位置
	int n = dir.size();
    if (dir.size() < n) {
        cout << path << " is not a file in dir " << dir << endl;
        exit(1);
    }
    for (int i = 0; i < n; i++) {
        if (dir[i] != path[i]) {
            cout << path << " is not a file in dir " << dir << endl;
            exit(1);
        }
    }
	return path.substr(n+1);
}

void mkdir_by_path(string path){
	string parent_path=get_parent_file(path);
	if(parent_path=="")return;
	if(get_parent_file(parent_path)!="")mkdir_by_path(parent_path);
	if(access(parent_path.c_str(),0)==-1){
		mkdir(parent_path.c_str(),S_IRWXU);
	}
}

void validate_args_num(int argc, int n) {
	if (argc != n) {
		cout << "Incorrect operands." << endl;
		exit(1);
	}
}