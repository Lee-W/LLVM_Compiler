#include<iostream>
#include<fstream>
#include<vector>
#include"Node.h"
#include"Symbol.h"

using namespace std;

//function prototype
void declaration(std::vector<Node>::iterator, vector<Node>);
vector<string> scanChildren (std::vector<Node>::iterator, vector<Node>);
void printID(std::vector<Node>::iterator);

void codeGeneration(vector<Node> parseTree, map<int, vector<Symbol>> symbolTable) 
{
	fstream llFile;
	llFile.open("output.ll",ios::out);
	
	//set up print function
	llFile << '@istr = private constant[4 x i8] c"%d\0A\00"' << endl;
	llFile << '@fstr = private constant[4 x i8] c"%f\0A\00"' << endl;
	llFile << "declare i32 @printf(i8*, ...)" << endl;
	/*
	// Some flag

	//scan the tree to generate the code
	for (std::vector<Node>::iterator it = parseTree.begin(); it != parseTree.end(); it++) 
	{

		//Start analyzing the tree
		if (it->symbol == "DeclList'")	//declaration of variable or function
			declaration(it, parseTree);
		else if (it->symbol == "VarDecl")	//declaration of variable (could be array)
			var_decl(it, parseTree);
		else if (it->symbol == "Stmt")	//statement is appeared
			statement(it, parseTree);
		else if (it->symbol == "{")
			
		else if (it->symbol == "}")		
	}*/
}

// global variable or function define
void declaration(std::vector<Node>::iterator it, vector<Node> parseTree)
{
	it = it + 2;
	string type = it->symbol;	//save the Type
	it = it + 2;
	string id = it->symbol;
	it = it + 2;
	//if (it->symbol == "VarDecl'")
		
}

//Scan downlevel node in order to know what's next
vector<string> scanChildren (std::vector<Node>::iterator it, vector<Node> parseTree)
{
	vector<string> children;
}

void statement(std::vector<Node>::iterator it, vector<Node> parseTree)
{
	std::vector<Node>::iterator temp = it++;
	if (temp->symbol == "print") {
		temp = temp + 2;		
		printID(temp);
	}
}

void printID(std::vector<Node>::iterator it)
{
	
}
