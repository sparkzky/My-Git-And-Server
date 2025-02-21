#include"Repository.h"

#include<iostream>
#include<cstring>

using namespace std;


int main(int argc,char**argv) {
	if (argc == 1) {
		cout << "Please enter a command." << endl;
		return 1;
	}
	string op = argv[1];

	SingleOperation* operation=Factory::create(op,argc,argv);
	operation->execute();

	return 0;
}

