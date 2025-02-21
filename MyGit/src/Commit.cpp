#include "../include/Commit.h"
#include "../include/Tools.h"
#include "../include/sha1.h"
#include "../include/Repository.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fstream>

using namespace std;

Commit::Commit() {
	date = get_time_to_set();
	message = "initial commit";
	id = generate_id();
	commit_file = generate_commit_file_route();
}

Commit::Commit(string message, vector<string> parents, unordered_map<string, string> blobs) {
	this->date = get_time_to_set();
	this->message = message;
	this->parents = parents;
	this->files_to_blobs = blobs;
	this->id = generate_id();
	this->commit_file = generate_commit_file_route();
}

string Commit::generate_id() {
	return SHA1::sha1(date, message, to_string(parents), to_string(files_to_blobs));
}

string Commit::generate_commit_file_route() {
	return join(Repository::OBJECTS_DIR, id);
}

string Commit::get_id() {
	return id;
}

unordered_map<string, string> Commit::get_files_to_blobs() {
	return files_to_blobs;
}

vector<string> Commit::get_parents() {
	return parents;
}

string Commit::get_message() {
	return message;
}

string Commit::get_date() {
	return date;
}
 
void Commit::save() {
	string dir = get_parent_file(commit_file);
	if (access(dir.c_str(), 0) == -1) {
		mkdir(dir.c_str(), S_IRWXU);
	}

	ofstream fout(commit_file);

	fout << "COMMIT\n";

	fout << message + '\n';

	fout << id + '\n';

	fout << parents.size() << '\n';
	for (auto it : parents) {
		fout << it + '\n';
	}

	fout << files_to_blobs.size() << '\n';
	for (auto it : files_to_blobs) {
		fout << get_relative_path(Repository::CWD, it.first) + '\n' + it.second + '\n';
	}

	fout << date + '\n';
	fout << get_relative_path(Repository::CWD, commit_file) + '\n';
	fout.close();
}

Commit Commit::read_commitfile_as_commit(string commit_file) {
	Commit commit;

	ifstream fin(commit_file);

	string line;
	getline(fin, line);
	if (line != "COMMIT"){
		throw "It's not a Commit";
	}

	getline(fin, commit.message);
	getline(fin, commit.id);

	getline(fin, line);
	int psize = atoi(line.c_str());//多少个父节点
	for (int i = 0; i < psize; i++) {
		getline(fin, line);
		commit.parents.push_back(line);
	}

	getline(fin, line);
	int bsize = atoi(line.c_str());//多少个blob
	for (int i = 0; i < bsize; i++) {
		getline(fin, line);
		string another;
		getline(fin, another);
		commit.files_to_blobs.insert({ join(Repository::CWD, line), another });
	}

	getline(fin, commit.date);
	getline(fin, commit.commit_file);
	commit.commit_file=join(Repository::CWD, commit.id);

	fin.close();
	return commit;
}

string Commit::get_time_to_set() {
	time_t now = time(0);
	string time = ctime(&now);
	return time;
}