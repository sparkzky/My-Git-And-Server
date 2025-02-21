#include "../include/Blob.h"
#include "../include/sha1.h"
#include "../include/Tools.h"
#include "../include/Repository.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fstream>

using namespace std;

Blob::Blob(string src){
	this->src_file = src;
	this->content = read_contents(src);
	this->id = SHA1::sha1(src, to_string(content));
	this->blob_file = join(Repository::OBJECTS_DIR, id);
}

Blob::Blob(string id, string src_file, string blob_file, vector<string> content){
	this->id = id;
	this->src_file = src_file;
	this->blob_file = blob_file;
	this->content = content;
} 

string Blob::get_id() {
	return id;
}

void Blob::save_to_blob() {
	string dir = get_parent_file(blob_file);
	if (access(dir.c_str(), 0) == -1) {
		mkdir(dir.c_str(), S_IRWXU);
	}

	ofstream fout(blob_file);
	fout << "BLOB\n";
	fout << id + '\n';
	fout << get_relative_path(Repository::CWD, src_file) + '\n';
	fout << get_relative_path(Repository::CWD, blob_file) + '\n';
	for (string line : content) {
		fout << line << '\n';
	}
	fout.close();
}

string Blob::get_content_as_string() {
	string text = "";
	for (string line : content) {
		text += text + '\n';
	}
	return text;
}

void Blob::save_to_src() {
	write_contents(src_file, content);
}

void Blob::save(){
	this->save_to_src();
	this->save_to_blob();
}

Blob Blob::read_blobfile_as_blob(string BlobFile) {
	ifstream fin(BlobFile);

	string line;
	getline(fin, line);
	if (line != "BLOB") {
		throw "It's not a Blob";
	}

	string id;
	getline(fin, id);
	string srcfile;
	getline(fin, srcfile);
	srcfile = join(Repository::CWD, srcfile);

	getline(fin, line);

	vector<string>content;
	while (getline(fin, line)) {
		content.push_back(line);
	}

	return Blob(id, srcfile, BlobFile, content);
}