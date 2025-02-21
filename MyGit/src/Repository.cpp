#include "../include/Repository.h"
#include "../include/Tools.h"
#include "../include/Blob.h"
#include"../include/Stage.h"
#include"../include/Client.h"
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>
#include <queue>
#include<filesystem>


using namespace std;

const string Repository::CWD = current_path().string();
const string Repository::MYGIT_DIR = join(Repository::CWD, ".mygit");
const string Repository::OBJECTS_DIR = join(Repository::MYGIT_DIR, "objects");
const string Repository::REFS_DIR = join(Repository::MYGIT_DIR, "refs");
const string Repository::HEADS_DIR = join(Repository::REFS_DIR, "heads");
const string Repository::HEAD = join(Repository::MYGIT_DIR, "HEAD");
const string Repository::STAGE = join(Repository::MYGIT_DIR, "stage");
const string Repository::USER=join(Repository::MYGIT_DIR, "user");
const string Repository::TREE=join(Repository::MYGIT_DIR, "tree");

void Repository::init() {
	if (access(MYGIT_DIR.c_str(), 0) != -1) {
		Exit("A MyGit version-control system already exists in the current directory.");
	}
	mkdir(MYGIT_DIR.c_str(), S_IRWXU);
	mkdir(OBJECTS_DIR.c_str(), S_IRWXU);
	mkdir(REFS_DIR.c_str(), S_IRWXU);
	mkdir(HEADS_DIR.c_str(), S_IRWXU);
	set_cur_branch("main");
	init_commit();
}

void Repository::add(string filename) {
	string file = join(CWD, filename);
	if (access(file.c_str(), 0) == -1) {
		Exit("File does not exist.");
	}
	Stage stage = read_stage();
	if (stage.add(file)) {
		stage.save();
	}
}

void Repository::remove(string filename) {
	string file = join(CWD, filename);
	Stage stage = read_stage();
	if (!stage.remove(file)) {
		Exit("Fail to remove the file.");
	}
}

void Repository::commit(string msg) {
	if (msg == "")
		Exit("Please enter a commit message.");

	Stage stage = Stage::read_stagefile_as_stage(STAGE);
	if (stage.is_empty())
		Exit("No changes add to commit");

	Commit cur_commit = get_cur_commit();

	unordered_map<string, string>files_to_blobs = cur_commit.get_files_to_blobs();
	unordered_map<string, string>add_object = stage.get_added_object();
	unordered_set<string>remove_object = stage.get_removed_object();

	files_to_blobs.insert(add_object.begin(), add_object.end());
	for (string file : remove_object) {
		files_to_blobs.erase(file);
	}

	vector<string>parents;
	parents.push_back(cur_commit.get_id());
	Commit commit(msg, parents, files_to_blobs);
	commit.save();

	add_commit(commit);
	//TODO 把stage文件的更改保存到TREE中
	Tree::save(stage);
	stage.clear();
}


void Repository::log() {
	Commit commit = get_cur_commit();
	while (!commit.get_parents().empty()) {
		print_commit(commit);
		vector<string>parents = commit.get_parents();
		commit = get_commit_by_id(parents[0]);
	}
	print_commit(commit);
}

void Repository::global_log() {
	vector<string>commit_list = plain_file_names_in(OBJECTS_DIR);
	for (string commit_id : commit_list) {
		try {
			Commit commit = get_commit_by_id(commit_id);
			print_commit(commit);
		}
		catch (...) {
			continue;
		}
	}
}

void Repository::find(string message) {
	vector<string>commit_list = plain_file_names_in(OBJECTS_DIR);
	bool got_it = false;
	for (string commit_id : commit_list) {
		try {
			Commit commit = get_commit_by_id(commit_id);
			if (message == commit.get_message()) {
				cout << commit_id << endl;
				got_it = true;
			}
		}
		catch (...) {
			continue;
		}
	}

	if (!got_it) {
		cout << "Found no commit with the message." << endl;
	}
}

void Repository::status() {
	print_branches();
	print_stage_files();
	print_removed_files();
	print_modifications_not_staged_for_commit();
	print_untracked();
}

void Repository::checkout(string file) {
	Commit cur_commit = get_cur_commit();
	checkout(cur_commit.get_id(), file);
}

void Repository::checkout(string commit_id, string filename) {
	Commit commit = get_commit_by_id(commit_id);

	unordered_map<string, string>files_to_blobs = commit.get_files_to_blobs();
	string file = join(CWD, filename);
	if (files_to_blobs.count(file)) {
		string blob_id = files_to_blobs[file];
		Blob blob = get_blob_by_id(blob_id);
		blob.save_to_src();
	}
	else {
		Exit("File does not exist in that commit.");
	}
}

void Repository::checkout_commit(string commit_id){
	Commit commit=get_commit_by_id(commit_id);
	unordered_map<string, string>files_to_blobs = commit.get_files_to_blobs();
	for(auto it:files_to_blobs){
		string blob_id=it.second;
		Blob blob=get_blob_by_id(blob_id);
		blob.save_to_src();
	}
}
//todo 用不用保存当前commit的
void Repository::checkout_branch(string branch_name) {
	string cur_branch = read_contents_as_string(HEAD);
	if (branch_name == cur_branch) {
		Exit("No need to checkout the current branch.");
	}
	vector<string>branches = plain_file_names_in(HEADS_DIR);
	if (std::count(branches.begin(), branches.end(), branch_name)==0)
		Exit("No such branch exists");

	Commit new_commit = get_head_commit_of_branch(branch_name);
	string new_commit_id = new_commit.get_id();
	change_commit_to(new_commit_id);

	set_cur_branch(branch_name);
}

void Repository::create_branch(string branch_name){
	if(branch_name.size()>=40){
		Exit("Branch name is too long.");
	}
	vector<string>branch_list = plain_file_names_in(HEADS_DIR);
	if (std::count(branch_list.begin(), branch_list.end(), branch_name)) {
		Exit("A branch with that name already exists");
	}
	string branch_file = join(HEADS_DIR, branch_name);
	Commit cur_commit = get_cur_commit();
	write_contents(branch_file, cur_commit.get_id());
}

void Repository::remove_branch(string branch_name) {
	string cur_branch = read_contents_as_string(HEAD);
	if (branch_name == cur_branch) {
		Exit("Cannot remove the current branch.");
	}
	vector<string>branches = plain_file_names_in(HEADS_DIR);
	if (std::count(branches.begin(), branches.end(), branch_name) == 0) {
		Exit("A branch with that name does not exist.");
	}
	string branch_file = join(HEADS_DIR, branch_name);
	std::remove(branch_file.c_str());
}

void Repository::reset(string commit_id) {
	change_commit_to(commit_id);
	string cur_branch = read_contents_as_string(HEAD);
	string branch_file = join(HEADS_DIR, cur_branch);
	write_contents(branch_file, commit_id);
}

void Repository::merge(string branch_name) {
	Stage stagearea = read_stage();
	if (!stagearea.is_empty()) {
		Exit("You have uncommited changes.");
	}

	vector<string>branches = plain_file_names_in(HEADS_DIR);
	if (std::count(branches.begin(), branches.end(), branch_name) == 0) {
		Exit("A branch with that name doesn't exist.");
	}

	string cur_branch = read_contents_as_string(HEAD);
	if (branch_name == cur_branch) {
		Exit("Cannot merge a branch with itself");
	}

	Commit cur_commit = get_cur_commit();
	Commit giv_commit = get_head_commit_of_branch(branch_name);
	Commit com_commit = get_common_commit(cur_commit, giv_commit);

	if (com_commit.get_id() == giv_commit.get_id()) {
		Exit("Given branch is an ancestor of the current branch.");
	}
	if (com_commit.get_id() == cur_commit.get_id()) {
		cout << "Current branch fast-forwarded." << endl;
		checkout_branch(branch_name);
		exit(0);
	}

	string message = "Merged" + branch_name + "into" + cur_branch;
	vector<string>parents;
	parents.push_back(cur_commit.get_id());
	parents.push_back(giv_commit.get_id());
	unordered_map<string, string>new_blobs = com_commit.get_files_to_blobs();

	unordered_map<string, string>cur_blobs = cur_commit.get_files_to_blobs();
	unordered_map<string, string>giv_blobs = giv_commit.get_files_to_blobs();
	unordered_map<string, string>com_blobs = com_commit.get_files_to_blobs();

	// for (auto it : giv_blobs) {
	// 	string file = it.first;
	// 	if (!cur_blobs.count(file) && !com_blobs.count(file)) {
	// 		string untracked_file = file;
	// 		if (access(untracked_file.c_str(), 0) != -1) {
	// 			Exit("There is an untracked file in the way; delete it, or add and commit it first.");
	// 		}
	// 	}
	// }

	bool got_a_conflict = false;
	unordered_set<string>all_files = get_all_files(cur_blobs, giv_blobs, com_blobs);
	for (string file : all_files) {
		if (cur_blobs.count(file) && giv_blobs.count(file) && com_blobs.count(file)) {
			string cur_blob_id = cur_blobs[file];
			string giv_blob_id = giv_blobs[file];
			string com_blob_id = com_blobs[file];
			if (cur_blob_id == com_blob_id && giv_blob_id != com_blob_id) {
				Blob blob = get_blob_by_id(giv_blob_id);
				blob.save_to_src();
				new_blobs.insert({ file,giv_blob_id });
				stagearea.add(file);
			}
			if (cur_blob_id != com_blob_id && giv_blob_id != com_blob_id) {
				string new_blob_id = deal_with_conflict(file, cur_blob_id,giv_blob_id);
				new_blobs.insert({ file,new_blob_id });
				got_a_conflict = true;
			}
		}
		if (!cur_blobs.count(file) && giv_blobs.count(file) && !com_blobs.count(file)) {
			string giv_blob_id = giv_blobs[file];
			Blob blob = get_blob_by_id(giv_blob_id);
			blob.save_to_src();
			new_blobs.insert({ file,giv_blob_id });
			stagearea.add(file);
		}
		if (cur_blobs.count(file) && !giv_blobs.count(file) && com_blobs.count(file)) {
			string cur_blob_id = cur_blobs[file];
			string com_blob_id = com_blobs[file];
			if (cur_blob_id == com_blob_id) {
				std::remove(file.c_str());
				new_blobs.erase(file);
			}
			else {
				string new_blob_id = deal_with_conflict(file, cur_blob_id, "");
				new_blobs.insert({ file,new_blob_id });
				got_a_conflict = true;
			}
		}
		if (cur_blobs.count(file) && giv_blobs.count(file) && !com_blobs.count(file)) {
			string cur_blob_id = cur_blobs[file];
			string giv_blob_id = giv_blobs[file];
			if (cur_blob_id != giv_blob_id) {
				string new_blob_id = deal_with_conflict(file, cur_blob_id, giv_blob_id);
				new_blobs.insert({ file,new_blob_id });
				got_a_conflict = true;
			}
		}
		if (!cur_blobs.count(file) && giv_blobs.count(file) && com_blobs.count(file)) {
			string com_blob_id = com_blobs[file];
			string giv_blob_id = giv_blobs[file];
			if (com_blob_id != giv_blob_id) {
				string new_blob_id = deal_with_conflict(file, com_blob_id, giv_blob_id);
				new_blobs.insert({ file,new_blob_id });
				got_a_conflict = true;
			}
		}
		if(cur_blobs.count(file) && !giv_blobs.count(file) && !com_blobs.count(file)){
			std::remove(file.c_str());
			new_blobs.erase(file);
		}

		Commit new_commit(message, parents, new_blobs);
		new_commit.save();
		add_commit(new_commit);
		Tree::save(stagearea);
		stagearea.clear();

		if (got_a_conflict)
			cout << "Encountered a merge conflict." << endl;
	}
}

void Repository::set_cur_branch(string branchname) {
	write_contents(HEAD, branchname);
}

Commit Repository::get_cur_commit() {
	string cur_branch = read_contents_as_string(HEAD);
	return get_head_commit_of_branch(cur_branch);
}

void Repository::check_if_init() {
	if (access(MYGIT_DIR.c_str(), 0) == -1) {
		Exit("Not in a initialized Git directory.");
	}
}

Commit Repository::get_head_commit_of_branch(string branchname) {
	string branch_head_file = join(HEADS_DIR, branchname);
	string branch_head_commit_id = read_contents_as_string(branch_head_file);
	string branch_head_commit_file = join(OBJECTS_DIR, branch_head_commit_id);
	Commit branch_head_commit = Commit::read_commitfile_as_commit(branch_head_commit_file);
	return branch_head_commit;
}



void Repository::add_commit(Commit commit) {
	string cur_branch = read_contents_as_string(HEAD);
	string cur_branch_head_file = join(HEADS_DIR, cur_branch);
	write_contents(cur_branch_head_file, commit.get_id());
}

void Repository::init_commit() {
	Commit commit;
	commit.save();
	add_commit(commit);
}

Commit Repository::get_commit_by_id(string id) {
	vector<string>commit_list = plain_file_names_in(OBJECTS_DIR);
	for (string commit_id : commit_list) {
		if(starts_with(commit_id,id)){
			id = commit_id;
			break;
		}
	}
	string commit_file = join(OBJECTS_DIR, id);
	if (access(commit_file.c_str(), 0) == -1) {
		Exit("No commit with that id exists.");
	}
	Commit commit = Commit::read_commitfile_as_commit(commit_file);
	return commit;
}

Blob Repository::get_blob_by_id(string id) {
	string blob_file = join(OBJECTS_DIR, id);
	if (access(blob_file.c_str(), 0) == -1) {
		exit(1);
	}
	Blob blob = Blob::read_blobfile_as_blob(blob_file);
	return blob;
}

void Repository::print_commit(Commit commit) {
	cout << "===" << endl;
	cout << "commit" << commit.get_id() << endl;
	if (commit.get_parents().size() > 1) {
		vector<string>parents = commit.get_parents();
		cout << "Merge" << parents[0].substr(0, 8) << " "
			<< parents[1].substr(0, 8) << endl;
	}
	cout << "Date: " << commit.get_date() << endl;
	cout << commit.get_message() << endl;
	cout << endl;
}

void Repository::print_branches() {
	vector<string>branch_list = plain_file_names_in(HEADS_DIR);
	string cur_branch = read_contents_as_string(HEAD);
	cout << "=== Branches ===" << endl;
	cout << "*" << cur_branch << endl;
	for (string branch : branch_list) {
		if (branch != cur_branch)
			cout << branch << endl;
	}
	cout << endl;
}

void Repository::print_stage_files() {
	cout << "=== Staged Files ===" << endl;
	Stage stage = read_stage();
	unordered_map<string, string>added = stage.get_added_object();
	vector<string>file_list;
	for (auto it : added) {
		file_list.push_back(it.first);
	}
	print_file_list(file_list);
}

void Repository::print_removed_files() {
	cout << "=== Removed Files ===" << endl;
	Stage stage = read_stage();
	unordered_set<string> removed = stage.get_removed_object();
	vector<string> file_list(removed.begin(), removed.end());
	print_file_list(file_list);
}

void Repository::print_modifications_not_staged_for_commit() {
	cout << "=== Modifications Not Staged For Commit ===" << endl;
	cout << endl;
}

void Repository::print_untracked() {
	cout << "=== Untracked Files ===" << endl;
	cout << endl;
}

void Repository::print_file_list(vector<string>file_list) {
	for (string file : file_list) {
		cout << get_filename(file) << endl;
	}
}

void Repository::change_commit_to(string commit_id) {
	Commit cur_commit = get_cur_commit();
	Commit new_commit = get_commit_by_id(commit_id);
	unordered_map<string, string>cur_blobs = cur_commit.get_files_to_blobs();
	unordered_map<string, string>new_blobs = new_commit.get_files_to_blobs();

	for (auto it : new_blobs) {//如果文件在现在commit中不存在，则为untracked文件
		string file = it.first;
		if (!cur_blobs.count(file)) {
			string untracked_file = file;
			if (access(untracked_file.c_str(), 0) != -1) {
				Exit("There is an untracked file in the way; delete it, or add and commit it first.");
			}
		}
	}

	for (auto it : cur_blobs) {//如果文件在新commit中不存在，则remove文件
		string file = it.first;
		if (!new_blobs.count(file)) {
			std::remove(file.c_str());
		}
	}

	for (auto it : new_blobs) {
		string blob_id = it.second;
		Blob blob = get_blob_by_id(blob_id);
		blob.save_to_src();
	}

	Stage stage_area = read_stage();
	//Tree::save(stage_area);
	stage_area.clear();
}

Commit Repository::get_common_commit(Commit a,Commit b){
	unordered_set<string>S;
	queue < Commit > Q;

	Q.push(a);
	while (!Q.empty()) {
		Commit commit = Q.front(); Q.pop();
		S.insert(commit.get_id());
		vector<string>parents = commit.get_parents();
		for (string parent : parents)
			Q.push(get_commit_by_id(parent));
	}

	Q.push(b);
	while (!Q.empty()) {
		Commit commit = Q.front(); Q.pop();
		string commit_id = commit.get_id();
		if (S.count(commit_id)) {
			return get_commit_by_id(commit_id);
		}
		else {
			S.insert(commit_id);
		}
		vector<string>parents = commit.get_parents();
		for (string parent : parents)
			Q.push(get_commit_by_id(parent));
	}

	return Commit();
}

unordered_set<string> Repository::get_all_files(unordered_map<string, string> a,
	unordered_map<string, string> b, unordered_map<string, string> c)
{
	unordered_set<string>S;
	for (auto it : a) {
		S.insert(it.first);
	}
	for (auto it : b) {
		S.insert(it.first);
	}
	for (auto it : c) {
		S.insert(it.first);
	}
	return S;
}

string Repository::deal_with_conflict(string file, string curBlobId, string givBlobId) {
	string cur_blob_content = (curBlobId != "") ? get_blob_by_id(curBlobId).get_content_as_string() : "";
	string giv_blob_content = (givBlobId != "") ? get_blob_by_id(givBlobId).get_content_as_string() : "";
	string merge_content = "<<<<<<<HEAD\n" + cur_blob_content + "\n=======\n"
		+ giv_blob_content + ">>>>>>>\n";
	write_contents(file, merge_content);

	Blob new_blob(file);
	new_blob.save();
	return new_blob.get_id();
}

Stage Repository::read_stage(){
	if(access(STAGE.c_str(),0)==-1){
		return Stage();
	}
	return Stage::read_stagefile_as_stage(STAGE);
}



void Repository::print_tree(){
	Tree::print_tree();
}
//todo 路径等等问题，可以先发送username

void Repository::clone(string path){
	string project=get_filename(path);
	mkdir(project.c_str(),S_IRWXU);
	string username=get_user_name();
	if(username==""){
		printf("Set username first");
		return;
	}
	
	Client client;
	string username_n_project=join(username,project);
	client.recv(username_n_project);

	Commit cur_commit=get_cur_commit();
	unordered_map<string,string>files_to_blobs=cur_commit.get_files_to_blobs();
	for(auto it:files_to_blobs){
		string blob_id=it.second;
		Blob blob=get_blob_by_id(blob_id);
		blob.save_to_src();
	}
}

void Repository::push(){
	string username=get_user_name();
	if(username==""){
		printf("Set username first");
		return;
	}
	string projectname=get_filename(CWD);

	Client client;
	client.send("username:"+username);
	client.send("project:"+projectname);

	queue<string>file_queue;
	file_queue.push(MYGIT_DIR);
	while(!file_queue.empty()){
		string file=file_queue.front();
		file_queue.pop();
		if(is_dir(file)){
			vector<string>files=plain_file_names_in(file);
			for(string f:files){
				file_queue.push(join(file,f));
			}
		}
		else{
			client.send_file(get_relative_path(CWD,file));
		}
	}
}


void Repository::set_user(string username){
	ofstream fout(USER);
	fout<<username;
	fout.close();
}

void Repository::help(){
    cout << "mygit init" << endl;
	cout << "mygit tree" << endl;
    cout << "mygit add [file name]" << endl;
    cout << "mygit remove [file name]" << endl;
    cout << "mygit commit [msg]" << endl;
    cout << "mygit log" << endl;
    cout << "mygit global-log" << endl;
    cout << "mygit find [message]" << endl;
    cout << "mygit status" << endl;
    cout << "mygit checkout -- [file name]" << endl;
    cout << "mygit checkout [commit id] -- [file name]" << endl;
    cout << "mygit checkout [commit id]" << endl;
    cout << "mygit checkout [branch name]" << endl;
    cout << "mygit branch" << endl;
    cout << "mygit branch [branch name]" << endl;
    cout << "mygit rm-branch branchName" << endl;
    cout << "mygit reset [commit]" << endl;
    cout << "mygit merge [branch name]" << endl;
    cout << "mygit push" << endl;
    cout << "mygit clone" << endl;
    cout << "mygit set username [username]" << endl;
    cout << "mygit --help" << endl;
}

string Repository::get_user_name(){
	ifstream fin(USER);
	if(fin){
		string username;
		fin>>username;
		fin.close();
		return username;
	}
	return "";
}

SingleOperation::SingleOperation(int argc,char**argv):argc(argc),argv(argv){}

SingleOperation* Factory::create(string op,int argc,char**argv){
	if(op=="init"){
		return new InitOperation(argc,argv);
	}
	else if(op=="tree"){
		return new PrintTreeOperation(argc,argv);
	}
	else if(op=="add"){
		return new AddOperation(argc,argv);
	}
	else if(op=="rm"){
		return new RemoveOperation(argc,argv);
	}
	else if (op=="commit"){
		return new CommitOperation(argc,argv);
	}
	else if(op=="log"){
		return new LogOperation(argc,argv);
	}
	else if(op=="global-log"){
		return new GlobalLogOperation(argc,argv);
	}
	else if(op=="find"){
		return new FindOperation(argc,argv);
	}
	else if(op=="status"){
		return new StatusOperation(argc,argv);
	}
	else if(op=="checkout"){
		Repository::check_if_init();
		switch(argc){
			case 4:
				return new CheckoutFileOperation(argc,argv);
			case 3:
				if(strlen(argv[2])==40){
					return new CheckoutCommitOperation(argc,argv);
				}
				else {
					return new CheckoutBranchOperation(argc,argv);
				}
			case 5:
				return new CheckoutCommitAndFileOperation(argc,argv);
			default:
				return new IncorrectOperands(argc,argv);
		}
	}
	else if(op=="branch"){
		Repository::check_if_init();
		switch(argc){
			case 2:
				return new PrintBranchesOperation(argc,argv);
			case 3:
				return new CreateBranchOperation(argc,argv);
			default:
				return new IncorrectOperands(argc,argv);
		}
	}
	else if(op=="rm-branch"){
		return new RemoveBranchOperation(argc,argv);
	}
	else if(op=="reset"){
		return new ResetOperation(argc,argv);
	}
	else if(op=="merge"){
		return new MergeOperation(argc,argv);
	}
	else if(op=="push"){
		return new PushOperation(argc,argv);
	}
	else if(op=="clone"){
		return new CloneOperation(argc,argv);
	}
	else if(op=="set"){
		return new SetUserOperation(argc,argv);
	}
	else if(op=="--help"){
		return new HelpOperation(argc,argv);
	}
	else{
		return new NullOperation(argc,argv);
	}
}

InitOperation::InitOperation(int argc,char**argv):SingleOperation(argc,argv){}
void InitOperation::execute(){
	validate_args_num(argc,2);
	Repository::init();
}

AddOperation::AddOperation(int argc,char**argv):SingleOperation(argc,argv){}
void AddOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,3);
	Repository::add(argv[2]);
}

RemoveOperation::RemoveOperation(int argc,char**argv):SingleOperation(argc,argv){}
void RemoveOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,3);
	Repository::remove(argv[2]);
}

CommitOperation::CommitOperation(int argc,char**argv):SingleOperation(argc,argv){}
void CommitOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,3);
	Repository::commit(argv[2]);
}

LogOperation::LogOperation(int argc,char**argv):SingleOperation(argc,argv){}
void LogOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,2);
	Repository::log();
}

GlobalLogOperation::GlobalLogOperation(int argc,char**argv):SingleOperation(argc,argv){}
void GlobalLogOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,2);
	Repository::global_log();
}

FindOperation::FindOperation(int argc,char**argv):SingleOperation(argc,argv){}
void FindOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,3);
	Repository::find(argv[2]);
}

StatusOperation::StatusOperation(int argc,char**argv):SingleOperation(argc,argv){}
void StatusOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,2);
	Repository::status();
}

CheckoutFileOperation::CheckoutFileOperation(int argc,char**argv):SingleOperation(argc,argv){}
void CheckoutFileOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,4);
	if(strcmp(argv[2],"--")){
		cout<<"Incorrect operands"<<endl;
		exit(1);
	}
	Repository::checkout(argv[3]);
}

CheckoutCommitAndFileOperation::CheckoutCommitAndFileOperation(int argc,char**argv):SingleOperation(argc,argv){}
void CheckoutCommitAndFileOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,5);
	if(strcmp(argv[3],"--")){
		cout<<"Incorrect operands"<<endl;
		exit(1);
	}
	Repository::checkout(argv[2],argv[4]);
}

CheckoutCommitOperation::CheckoutCommitOperation(int argc,char**argv):SingleOperation(argc,argv){}
void CheckoutCommitOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,3);
	Repository::checkout_commit(argv[2]);
}

CheckoutBranchOperation::CheckoutBranchOperation(int argc,char**argv):SingleOperation(argc,argv){}
void CheckoutBranchOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,3);
	Repository::checkout_branch(argv[2]);
}

IncorrectOperands::IncorrectOperands(int argc,char**argv):SingleOperation(argc,argv){}
void IncorrectOperands::execute(){
	cout<<"Incorrect operands"<<endl;
	exit(1);
}


PrintTreeOperation::PrintTreeOperation(int argc,char**argv):SingleOperation(argc,argv){}
void PrintTreeOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,2);
	Repository::print_tree();
}

PrintBranchesOperation::PrintBranchesOperation(int argc,char**argv):SingleOperation(argc,argv){}
void PrintBranchesOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,2);
	Repository::print_branches();
}

CreateBranchOperation::CreateBranchOperation(int argc,char**argv):SingleOperation(argc,argv){}
void CreateBranchOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,3);
	Repository::create_branch(argv[2]);
}


RemoveBranchOperation::RemoveBranchOperation(int argc,char**argv):SingleOperation(argc,argv){}
void RemoveBranchOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,3);
	Repository::remove_branch(argv[2]);
}


ResetOperation::ResetOperation(int argc,char**argv):SingleOperation(argc,argv){}
void ResetOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,3);
	Repository::reset(argv[2]);
}


MergeOperation::MergeOperation(int argc,char**argv):SingleOperation(argc,argv){}
void MergeOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,3);
	Repository::merge(argv[2]);
}


PushOperation::PushOperation(int argc,char**argv):SingleOperation(argc,argv){}
void PushOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,2);
	Repository::push();
}


CloneOperation::CloneOperation(int argc,char**argv):SingleOperation(argc,argv){}
void CloneOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,3);
	Repository::clone(argv[2]);
}


SetUserOperation::SetUserOperation(int argc,char**argv):SingleOperation(argc,argv){}
void SetUserOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,4);
	if(strcmp(argv[2],"username")){
		cout<<"Incorrect operands"<<endl;
		exit(1);
	}
	Repository::set_user(argv[3]);
}


HelpOperation::HelpOperation(int argc,char**argv):SingleOperation(argc,argv){}
void HelpOperation::execute(){
	Repository::check_if_init();
	validate_args_num(argc,2);
	Repository::help();
}


NullOperation::NullOperation(int argc,char**argv):SingleOperation(argc,argv){}
void NullOperation::execute(){
	cout<<"No such operation"<<endl;
	exit(1);
}