#include<iostream>
#include<stdio.h>
//#include<fstream>
#include<vector>
#include<map>
#include"Node.h"
#include"Symbol.h"

using namespace std;

FILE* llFile;

const char* type_cast(string type){
	string ret;
	if (type == "int")
		ret = "i32";
	else if (type == "double")
		ret = "double";
	else if (type == "char")
		ret = "i8";
	else
		printf("Unknown Type found!\n");
	return ret.c_str();
}


void printID(std::vector<Node>::iterator it)
{
	
}

// global variable or function define
void declaration(std::vector<Node>::iterator it)
{
	it = it + 2;
	string type = it->symbol;	//save the Type
	it = it + 2;
	string id = it->symbol;
	it = it + 2;
	
	
	if (it->symbol == "VarDecl'") {		//global variable/array declaration
		it++;
		if (it->symbol == ";") {	//global variable
			if (type == "int")
				fprintf(llFile, "@%s = global %s 0\n", id.c_str(), type_cast(type));
			else if (type == "double")
				fprintf(llFile, "@%s = global %s 0.000000e+00\n", id.c_str(), type_cast(type));
			else if (type == "char")
				fprintf(llFile, "@%s = global %s 0\n", id.c_str(), type_cast(type));
		}
		else if (it->symbol == "[") {	//global array
			it = it + 2;
			fprintf(llFile, "@%s = global [%s x %s] zeroinitializer\n", id.c_str(), it->symbol.c_str(), type_cast(type));
		}
	}
	else if (it->symbol == "FunDecl") {		//function declaration
		it = it + 3;
		if (it->symbol == "epsilon")	//No parameters
			fprintf(llFile, "define %s @%s() ", type_cast(type), id.c_str());
	}
}

void var_decl(std::vector<Node>::iterator it) {		//local variable declartion
	it = it + 2;
	string type = it->symbol;	//save the Type
	it = it + 2;
	string id = it->symbol;		//save the id
	it = it + 2;
	if (it->symbol == ";") {	//variable
		fprintf(llFile, "%%%s = alloca %s\n", id.c_str(), type_cast(type));
	}
	else if (it->symbol == "[") {	//array
		it = it + 2;
		fprintf(llFile, "%%%s = alloca [%s x %s]\n", id.c_str(), it->symbol.c_str(), type_cast(type));
	}
}

/*
//Scan downlevel node in order to know what's next
vector<string> scanChildren (std::vector<Node>::iterator it)
{
	vector<string> children;
}*/

void statement(std::vector<Node>::iterator it, map<int, vector<Symbol>> symbolTable)
{
	std::vector<Node>::iterator temp = it++;
	if (temp->symbol == "print") {
		temp = temp + 2;		
		printID(temp);
	}
}

void codeGeneration(vector<Node> parseTree, map<int, vector<Symbol>> symbolTable) 
{

	llFile = fopen("output.ll","w");
	
	//set up print function
	fprintf(llFile, "@istr = private constant[4 x i8] c\"%%d\\0A\\00\"\n");
	fprintf(llFile, "@fstr = private constant[4 x i8] c\"%%f\\0A\\00\"\n");
	fprintf(llFile, "declare i32 @printf(i8*, ...)\n\n");
	
	// Some flag

	//scan the tree to generate the code
	for (std::vector<Node>::iterator it = parseTree.begin(); it != parseTree.end(); it++) 
	{
		//Start analyzing the tree
		if (it->symbol == "DeclList'")	//declaration of variable or function
			declaration(it);
		else if (it->symbol == "VarDecl")	//declaration of variable (could be array)
			var_decl(it);
		else if (it->symbol == "Stmt")	//statement is appeared
			statement(it, symbolTable);
		else if (it->symbol == "{")
			fprintf(llFile, "{\n");
		else if (it->symbol == "}")		
			fprintf(llFile, "}\n");
	}
	fclose(llFile);
}
