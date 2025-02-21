#include "../include/Stage.h"
#include "../include/Repository.h"
#include "../include/Commit.h"

#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

unordered_map<string, string>Stage::get_added_object() {
	return added_object;
}

unordered_set<string>Stage::get_removed_object() {
	return removed_object;
}

void Stage::save() {
	ofstream fout(Repository::STAGE);
	fout << added_object.size() << '\n';
	for (auto it : added_object) {
		fout << it.first << '\n' << it.second << '\n';
	}
	fout << removed_object.size() << '\n';
	for(auto it:removed_object){
		fout << it << '\n';
	}
	fout.close();
}

bool Stage::add(string file) {
	if(access(file.c_str(),0) == -1){
		printf("file %s not exist\n",file.c_str());
		return false;
	}
	Blob blob(file);
	if (added_object.count(file)&& added_object[file] == blob.get_id()) //已经存在该blob文件并且没有更改
		return false;

	if (is_new_blob(blob, file)) {
		added_object.insert({ file,blob.get_id() });
	}
	removed_object.erase(file);
	blob.save_to_blob();//保存blob文件
	save();//保存到stage文件
	return true;
}

bool Stage::remove(string file) {
	if (added_object.count(file)) {
		added_object.erase(file);
		save();
		return true;
	}

	Commit cur_commit = Repository::get_cur_commit();
	if (cur_commit.get_files_to_blobs().count(file)) {
		if (access(file.c_str(),0) != -1)
			std::remove(file.c_str());
		removed_object.insert(file);
		save();
		return true;
	}

	return false;
}

bool Stage::is_empty() {
	return added_object.empty() && removed_object.empty();
}

void Stage::clear() {
	added_object.clear();
	removed_object.clear();
	save();
}

Stage Stage::read_stagefile_as_stage(string stagefile) {
	Stage stage;
	ifstream fin(stagefile);
	string line;

	getline(fin, line);
	int asize = atoi(line.c_str());
	for (int i = 0; i < asize; i++) {
		getline(fin, line);
		string another_line;
		getline(fin, another_line);
		stage.added_object.insert({ line, another_line });
	}

	getline(fin, line);
	int rsize = atoi(line.c_str());
	for (int i = 0; i < rsize; i++) {
		getline(fin, line);
		stage.removed_object.insert(line);
	}
	fin.close();

	return stage;
}

bool Stage::is_new_blob(Blob blob, string path) {
	Commit cur_commit = Repository::get_cur_commit();
	if (cur_commit.get_files_to_blobs().count(path) && cur_commit.get_files_to_blobs()[path] == blob.get_id()) {
		return false;
	}
	return true;
}