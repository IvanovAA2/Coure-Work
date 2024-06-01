

#if !defined(COCO_PARSER_H__)
#define COCO_PARSER_H__

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <unordered_map>
#include <set>
#include <list>
#include <stack>
#include <vector>
#include <cstdlib>
#include <cstddef>

#include <utility>
#include <chrono>

using namespace std;


#include "Scanner.h"



class Errors {
public:
	int count;			// number of errors detected

	Errors();
	void SynErr(int line, int col, int n);
	void Error(int line, int col, const wchar_t *s);
	void Warning(int line, int col, const wchar_t *s);
	void Warning(const wchar_t *s);
	void Exception(const wchar_t *s);

}; // Errors

class Parser {
private:
	enum {
		_EOF=0,
		_ID=1,
		_NUMBER=2,
		_EQUALSIGN=3,
		_ADD_EQUALSIGN=4,
		_SUB_EQUALSIGN=5,
		_MUL_EQUALSIGN=6,
		_DIV_EQUALSIGN=7,
		_REM_EQUALSIGN=8,
		_ENDL=9,
		_DEL=10,
		_VAR=11,
		_BOOL=12,
		_INT=13,
		_FLOAT=14
	};
	int maxT;

	Token *dummyToken;
	int errDist;
	int minErrDist;

	void SynErr(int n);
	void Get();
	void Expect(int n);
	bool StartOf(int s);
	void ExpectWeak(int n, int follow);
	bool WeakSeparator(int n, int syFol, int repFol);

public:
	Scanner *scanner;
	Errors  *errors;

	Token *t;			// last recognized token
	Token *la;			// lookahead token

enum Type : uint8_t {
	
	bool_t, 
	int_t, 
	float_t,
	
	pointer_t,
	
	size_of_type
};

enum Operation : uint8_t {
	op_start = size_of_type,
	
	not_op,
	
	or_op,
	and_op,
	
	int_int_bit_or_op,
	int_int_bit_xor_op,
	int_int_bit_and_op,
	
	bool_bool_eq_op,
	int_int_eq_op,
	float_float_eq_op,
	int_float_eq_op,
	float_int_eq_op,
	
	bool_bool_uneq_op,
	int_int_uneq_op,
	float_float_uneq_op,
	int_float_uneq_op,
	float_int_uneq_op,
	
	int_int_ls_op,
	float_float_ls_op,
	int_float_ls_op,
	float_int_ls_op,
	
	int_int_gr_op,
	float_float_gr_op,
	int_float_gr_op,
	float_int_gr_op,
	
	int_int_lseq_op,
	float_float_lseq_op,
	int_float_lseq_op,
	float_int_lseq_op,
	
	int_int_greq_op,
	float_float_greq_op,
	int_float_greq_op,
	float_int_greq_op,
	
	int_int_add_op,
	float_float_add_op,
	int_float_add_op,
	float_int_add_op,
	
	int_int_sub_op,
	float_float_sub_op,
	int_float_sub_op,
	float_int_sub_op,
	
	int_int_mul_op,
	float_float_mul_op,
	int_float_mul_op,
	float_int_mul_op,
	
	int_int_div_op,
	float_float_div_op,
	int_float_div_op,
	float_int_div_op,
	
	int_int_rem_op,
	
	add_val_op,
	pop_val_op,
	
	bool_bool_asgn_op,
	int_int_asgn_op,
	int_float_asgn_op,
	float_float_asgn_op,
	float_int_asgn_op,
	
	int_int_add_asgn_op,
	int_float_add_asgn_op,
	float_float_add_asgn_op,
	float_int_add_asgn_op,
	
	int_int_sub_asgn_op,
	int_float_sub_asgn_op,
	float_float_sub_asgn_op,
	float_int_sub_asgn_op,
	
	int_int_mul_asgn_op,
	int_float_mul_asgn_op,
	float_float_mul_asgn_op,
	float_int_mul_asgn_op,
	
	int_int_div_asgn_op,
	int_float_div_asgn_op,
	float_float_div_asgn_op,
	float_int_div_asgn_op,
	
	int_int_rem_asgn_op,
	
	bool_print_op,
	int_print_op,
	float_print_op,
	
	print_del_op,
	print_endl_op,
	
	if_op,
	else_op,
	while_op,
	jump_op,
	
	int_to_bool_op,
	float_to_bool_op,
	bool_to_int_op,
	float_to_int_op,
	bool_to_float_op,
	int_to_float_op
} ;

struct Op {
	Operation opr;
	pointert ptr, op1, op2;
};

struct Var {
	pointert ptr;
	Type type;
};

static vector<Type> types;

static vector<Op> operations;

static vector<uint8_t> tape;

static vector<pointert> unused;

static size_t scope_level;

static map<wstring, Var> ident_table;

static unordered_map<wstring, Var> global_scope;

static stack<list<wstring>> to_remove;

//----------------------------------------------------------

inline static void reserve(size_t size) {
	for(size_t i = 0; i < size; ++i) {
		tape.push_back(-1);
	}
}

inline static pointert choose_ident(const wchar_t* name, Type type) {
	wstring sname(name);
	if(ident_table.contains(sname)){
		wcout << L"variable " << sname << L" already exists" << endl;
		exit(1);
	}
	pointert ptr = size(tape);
	ident_table[name] = {ptr, type};
	switch(type){
		case bool_t: 	
			reserve(sizeof(bool));
			break;
		case int_t: 	
			reserve(sizeof(intt));
			break;
		case float_t: 
			reserve(sizeof(floatt));
			break;
	}
	return ptr;
}

inline static Var get_ident(const wchar_t* name){
	if( ident_table.contains(wstring(name))) {
		return ident_table.at(wstring(name));
	}
	wcout << "variable " << name << " does not exist" << endl;
	exit(1);
}

//----------------------------------------------------------

static void prep() {
	tape.shrink_to_fit();
	operations.shrink_to_fit();
	for(Op& x: operations) {
		static set<Operation> not_conv = {if_op, else_op, while_op, jump_op};
		if(not_conv.contains(x.opr) == false) {
			x.ptr = pointert(data(tape) + x.ptr);
		}
		if(x.op1 == -1) {
			x.op1 = 0;
		}
		else {
			x.op1 = pointert(data(tape) + x.op1);
		}
		if (x.op2 == -1) {
			x.op2 = 0;
		}
		else {
			x.op2 = pointert(data(tape) + x.op2);
		}
	}
}

static void run() {
	
	size_t cur_opr = 0;
	
	while(cur_opr < size(operations)) {
		
		auto [opr, ptr, op1, op2] = operations[cur_opr];
		//uint64_t b = uint64_t(data(tape));
		//unused;
		//tape;
		
		switch(opr) {

			case not_op: {
				*(bool*)ptr = not *(const bool*)op1;
			} break;

			case or_op: {
				*(bool*)ptr = *(const bool*)op1 or *(const bool*)op2;
			} break;
				
			case and_op: {
				*(bool*)ptr = *(const bool*)op1 and *(const bool*)op2;
			} break;
			
			case int_int_bit_and_op: {
				*(intt*)ptr = *(const intt*)op1 & *(const intt*)op2;
			} break;
				
			case int_int_bit_or_op: {
				*(intt*)ptr = *(const intt*)op1 | *(const intt*)op2;
			} break;
				
			case int_int_bit_xor_op: {
				*(intt*)ptr = *(const intt*)op1 ^ *(const intt*)op2;
			} break;
				
			case bool_bool_eq_op: {
				*(bool*)ptr = *(const bool*)op1 == *(const bool*)op2;
			} break;

			case int_int_eq_op: {
				*(bool*)ptr = *(const intt*)op1 == *(const intt*)op2;
			} break;

			case float_float_eq_op: {
				*(bool*)ptr = *(const floatt*)op1 == *(const floatt*)op2;
			} break;

			case bool_bool_uneq_op: {
				*(bool*)ptr = *(const bool*)op1 != *(const bool*)op2;
			} break;

			case int_int_uneq_op: {
				*(bool*)ptr = *(const intt*)op1 != *(const intt*)op2;
			} break;

			case float_float_uneq_op: {
				*(bool*)ptr = *(const floatt*)op1 != *(const floatt*)op2;
			} break;

			case int_int_ls_op: {
				*(bool*)ptr = *(const intt*)op1 < *(const intt*)op2;
			} break;

			case float_float_ls_op: {
				*(bool*)ptr = *(const floatt*)op1 < *(const floatt*)op2;
			} break;

			case int_float_ls_op: {
				*(bool*)ptr = *(const intt*)op1 < *(const floatt*)op2;
			} break;

			case float_int_ls_op: {
				*(bool*)ptr = *(const floatt*)op1 < *(const intt*)op2;
			} break;

			case int_int_gr_op: {
				*(bool*)ptr = *(const intt*)op1 > *(const intt*)op2;
			} break;

			case float_float_gr_op: {
				*(bool*)ptr = *(const floatt*)op1 > *(const floatt*)op2;
			} break;

			case int_float_gr_op: {
				*(bool*)ptr = *(const intt*)op1 > *(const floatt*)op2;
			} break;

			case float_int_gr_op: {
				*(bool*)ptr = *(const floatt*)op1 > *(const intt*)op2;
			} break;

			case int_int_lseq_op: {
				*(bool*)ptr = *(const intt*)op1 <= *(const intt*)op2;
			} break;

			case float_float_lseq_op: {
				*(bool*)ptr = *(const floatt*)op1 <= *(const floatt*)op2;
			} break;

			case int_float_lseq_op: {
				*(bool*)ptr = *(const intt*)op1 <= *(const floatt*)op2;
			} break;

			case float_int_lseq_op: {
				*(bool*)ptr = *(const floatt*)op1 <= *(const intt*)op2;
			} break;

			case int_int_greq_op: {
				*(bool*)ptr = *(const intt*)op1 >= *(const intt*)op2;
			} break;

			case float_float_greq_op: {
				*(bool*)ptr = *(const floatt*)op1 >= *(const floatt*)op2;
			} break;

			case int_float_greq_op: {
				*(bool*)ptr = *(const intt*)op1 >= *(const floatt*)op2;
			} break;

			case float_int_greq_op: {
				*(bool*)ptr = *(const floatt*)op1 >= *(const intt*)op2;
			} break;

			case int_int_add_op: {
				*(intt*)ptr = *(const intt*)op1 + *(const intt*)op2;
			} break;

			case float_float_add_op: {
				*(floatt*)ptr = *(const floatt*)op1 + *(const floatt*)op2;
			} break;

			case int_float_add_op: {
				*(floatt*)ptr = *(const intt*)op1 + *(const floatt*)op2;
			} break;
			
			case float_int_add_op: {
				*(floatt*)ptr = *(const floatt*)op1 + *(const intt*)op2;
			} break;

			case int_int_sub_op: {
				*(intt*)ptr = *(const intt*)op1 - *(const intt*)op2;
			} break;

			case float_float_sub_op: {
				*(floatt*)ptr = *(const floatt*)op1 - *(const floatt*)op2;
			} break;

			case int_float_sub_op: {
				*(floatt*)ptr = *(const intt*)op1 - *(const floatt*)op2;
			} break;
			
			case float_int_sub_op: {
				*(floatt*)ptr = *(const floatt*)op1 - *(const intt*)op2;
			} break;

			case int_int_mul_op: {
				*(intt*)ptr = *(const intt*)op1 * *(const intt*)op2;
			} break;

			case float_float_mul_op: {
				*(floatt*)ptr = *(const floatt*)op1 * *(const floatt*)op2;
			} break;

			case int_float_mul_op: {
				*(floatt*)ptr = *(const intt*)op1 * *(const floatt*)op2;
			} break;
			
			case float_int_mul_op: {
				*(floatt*)ptr = *(const floatt*)op1 * *(const intt*)op2;
			} break;

			case int_int_div_op: {
				*(intt*)ptr = *(const intt*)op1 / *(const intt*)op2;
			} break;

			case float_float_div_op: {
				*(floatt*)ptr = *(const floatt*)op1 / *(const floatt*)op2;
			} break;

			case int_float_div_op: {
				*(floatt*)ptr = *(const intt*)op1 / *(const floatt*)op2;
			} break;
			
			case float_int_div_op: {
				*(floatt*)ptr = *(const floatt*)op1 / *(const intt*)op2;
			} break;

			case int_int_rem_op: {
				*(intt*)ptr = *(const intt*)op1 % *(const intt*)op2;
			} break;
				
			case bool_bool_asgn_op: {
				*(bool*)ptr = *(const bool*)op1;
			} break;

			case int_int_asgn_op: {
				*(intt*)ptr = *(const intt*)op1;
			} break;

			case int_float_asgn_op: {
				*(intt*)ptr = *(const floatt*)op1;
			} break;

			case float_int_asgn_op: {
				*(floatt*)ptr = *(const intt*)op1;
			} break;

			case float_float_asgn_op: {
				*(floatt*)ptr = *(const floatt*)op1;
			} break;

			case int_int_add_asgn_op: {
				*(intt*)ptr += *(const intt*)op1;
			} break;

			case int_float_add_asgn_op: {
				*(intt*)ptr += *(const floatt*)op1;
			} break;

			case float_float_add_asgn_op: {
				*(floatt*)ptr += *(const floatt*)op1;
			} break;

			case float_int_add_asgn_op: {
				*(floatt*)ptr += *(const intt*)op1;
			} break;

			case int_int_sub_asgn_op: {
				*(intt*)ptr -= *(const intt*)op1;
			} break;

			case int_float_sub_asgn_op: {
				*(intt*)ptr -= *(const floatt*)op1;
			} break;

			case float_float_sub_asgn_op: {
				*(floatt*)ptr -= *(const intt*)op1;
			} break;

			case float_int_sub_asgn_op: {
				*(floatt*)ptr -= *(const intt*)op1;
			} break;
			
			case int_int_mul_asgn_op: {
				*(intt*)ptr *= *(const intt*)op1;
			} break;

			case int_float_mul_asgn_op: {
				*(intt*)ptr *= *(const floatt*)op1;
			} break;

			case float_float_mul_asgn_op: {
				*(floatt*)ptr *= *(const floatt*)op1;
			} break;

			case float_int_mul_asgn_op: {
				*(floatt*)ptr *= *(const intt*)op1;
			} break;
			
			case int_int_div_asgn_op: {
				*(intt*)ptr /= *(const intt*)op1;
			} break;

			case int_float_div_asgn_op: {
				*(intt*)ptr /= *(const floatt*)op1;
			} break;

			case float_float_div_asgn_op: {
				*(floatt*)ptr /= *(const floatt*)op1;
			} break;

			case float_int_div_asgn_op: {
				*(floatt*)ptr /= *(const intt*)op1;
			} break;
			
			case int_int_rem_asgn_op: {
				*(intt*)ptr %= *(const intt*)op1;
			} break;
			
			case bool_print_op: {
				cout << (*(bool*)op1 ? "True" : "False");
			} break;

			case int_print_op: {
				cout << *(intt*)op1;
			} break;

			case float_print_op: {
				cout << *(floatt*)op1;
			} break;

			case print_del_op: {
				cout << " ";
			} break;

			case print_endl_op: {
				cout << "\n";
			} break;

			case if_op: {
				if(*(bool*)op1 == false) {
					cur_opr = ptr - 1;
				}
			} break;

			case else_op: {
				if(*(bool*)op1) {
					cur_opr = ptr - 1;
				}
			} break;

			case while_op: {
				if(*(bool*)op1 == false) {
					cur_opr = ptr - 1;
				}
			} break;

			case jump_op: {
				cur_opr = ptr - 1;
			} break;
				
			case int_to_bool_op: {
				*(bool*)ptr = *(intt*)op1;
			} break;

			case float_to_bool_op: {
				*(bool*)ptr = *(floatt*)op1;
			} break;

			case bool_to_int_op: {
				*(intt*)ptr = *(bool*)op1;
			} break;

			case float_to_int_op: {
				*(intt*)ptr = *(floatt*)op1;
			} break;

			case bool_to_float_op: {
				*(floatt*)ptr = *(bool*)op1;
			} break;

			case int_to_float_op: {
				*(floatt*)ptr = *(intt*)op1;
			} break;
			
		}
		
		++cur_opr;
	}
}

//--------------------------------------------------

inline static void add_bool(bool val) {
	unused.push_back(size(tape));
	reserve(sizeof(bool));
	*(bool*)(data(tape) + size(tape) - sizeof(bool)) = val;
	types.push_back(bool_t);
}

inline static void add_int(intt val) {
	unused.push_back(size(tape));
	reserve(sizeof(intt));
	*(intt*)(data(tape) + size(tape) - sizeof(intt)) = val;
	types.push_back(int_t);
}

inline static void add_float(floatt val) {
	unused.push_back(size(tape));
	reserve(sizeof(floatt));
	*(floatt*)(data(tape) + size(tape) - sizeof(floatt)) = val;
	types.push_back(float_t);
}

inline static void add_val(pointert ptr, Type type) {
	unused.push_back(ptr);
	add_type(type);
}

inline static void pop_val(int amount = 1) {
	for (int i = 0; i < amount; i++) {
		unused.pop_back();
		types.pop_back();
	}
}

inline static void add_op(Operation opr, pointert ptr = -2, pair<pointert, pointert> op = {-1, -1}) {
	if(ptr == -1){ 
		ptr = size(tape);
		switch(types.back()){
		case bool_t: 	
			reserve(sizeof(bool));
			break;
		case int_t: 	
			reserve(sizeof(intt));
			break;
		case float_t: 
			reserve(sizeof(floatt));
			break;
		}
	}
	unused.push_back(ptr);
	operations.emplace_back(opr, ptr, op.first, op.second);
}

inline static void add_type(Type type) {
	types.push_back(type);
}

inline static void switch_type(Type type) {
	types.back() = type;
}

inline static Type pop_type() {
	Type res = types.back();
	types.pop_back();
	return res;
}

inline static void add_var(Var x) {
	unused.push_back(x.ptr);
	types.push_back(x.type);
}

inline static pair<pointert, pointert> get_val() {
	pair<pointert, pointert> res;
	res.first = unused.back();
	unused.pop_back();
	res.second = -1;
	return res;
}

inline static pair<pointert, pointert> get_vals() {
	pair<pointert, pointert> res;
	res.second = unused.back();
	unused.pop_back();
	res.first = unused.back();
	unused.pop_back();
	return res;
}

//--------------------------------------------------

// LL(1) conflic resolution functions

bool is_declaration() 
{
	static vector<int> types = {
		_VAR, 
		_BOOL,
		_INT,
		_FLOAT
	};
    return 
    find(begin(types), end(types), int(la->kind)) != end(types) and 
    scanner->Peek()->kind == _ID;
}

bool is_assignment() 
{
	if(la->kind != _ID){
		return false;
	}
    auto kind = scanner->Peek()->kind;
    return 
	kind ==     _EQUALSIGN or 
	kind == _ADD_EQUALSIGN or 
	kind == _SUB_EQUALSIGN or 
	kind == _MUL_EQUALSIGN or 
	kind == _DIV_EQUALSIGN or 
	kind == _REM_EQUALSIGN;
}



	Parser(Scanner *scanner);
	~Parser();
	void SemErr(const wchar_t* msg);

	void Program();
	void Statements();
	void Statement();
	void VariableDeclaration();
	void Expression();
	void ProcCall();
	void Conditional();
	void Loop();
	void Scope();
	void SingleDeclaration(Type type);
	void If();
	void While();
	void For();
	void Assignment();
	void Expr();
	void BoolTerm();
	void BitOr();
	void BitXor();
	void BitAnd();
	void BoolEq();
	void BoolComp();
	void ArExpr();
	void Term();
	void Value();
	void Boolean();
	void Number();
	void FuncCall();
	void Variable();
	void DefFuncCall();
	void DefProcCall();
	void Print();
	void Println();

	void Parse();

}; // end Parser



// MODIFIED !!!

void skip_space();

void unskip_space();
//

#endif

