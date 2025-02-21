#include "../include/Tree.h"
#include "../include/Stage.h"
#include "../include/Repository.h"
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std;


void Tree::print_tree(){
    ifstream fin(Repository::TREE);
    string line;
    while(getline(fin, line)){
        cout<<line<<endl;
    }
    fin.close();
}

void Tree::save(const Stage& stage){
    ifstream fin(Repository::STAGE);
    ofstream fout(Repository::TREE, ofstream::app);

    string line;
    fout<<"COMMITED ON: "<<get_time_to_set();
    while(getline(fin, line)){
        fout<<line<<endl;
    }
    fout<<"====END OF CURRENT COMMIT===="<<endl;
    fin.close();
    fout.close();
}

void Tree::clear(){
    ofstream fout(Repository::TREE);
    fout<<"";
    fout.close();
}

bool Tree::is_empty(){
    ifstream fin(Repository::TREE);
    string line;
    getline(fin, line);
    fin.close();

    if(line.empty()){
        return true;
    }
    else{
        return false;
    }
}

string Tree::get_time_to_set(){
    time_t now = time(0);
	string time = ctime(&now);
	return time;
}