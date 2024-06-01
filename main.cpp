#include <iostream>
#include <wchar.h>
#include "Parser.h"
#include "Scanner.h"
 
#include <string>

bool _skip_space = true; 

double cmp_time;

double run_time;

vector<Parser::Type> Parser::types;

vector<Parser::Op> Parser::operations;

vector<uint8_t> Parser::tape;

vector<pointert> Parser::unused;

map<wstring, Parser::Var> Parser::ident_table;
 
using namespace std;
 
int main(int argc, char* argv[])
{
	if (argc == 1) {
		cout << "Use: translator filename" << endl;
		return 1;
	}
	wchar_t* file = coco_string_create(argv[1]);

	Scanner scanner(file);
	Parser parser(&scanner);
	parser.Parse();
	
	if(argc == 3){
		string s(argv[2]);
		if(s.find('t') != -1){
			cout << fixed << setprecision(2) << "compilation: " << cmp_time << endl;
			cout << fixed << setprecision(2) << "runtime:     " << run_time << endl;;
		}
	}

	delete file;

	return 0;
}