

#include <wchar.h>
#include "Parser.h"
#include "Scanner.h"

// MODIFIED !!!

void skip_space() {
	_skip_space = true;
}

void unskip_space() {
	_skip_space = false;
}

//



void Parser::SynErr(int n) {
	if (errDist >= minErrDist) errors->SynErr(la->line, la->col, n);
	errDist = 0;
}

void Parser::SemErr(const wchar_t* msg) {
	if (errDist >= minErrDist) errors->Error(t->line, t->col, msg);
	errDist = 0;
}

void Parser::Get() {
	for (;;) {
		t = la;
		la = scanner->Scan();
		if (la->kind <= maxT) { ++errDist; break; }

		if (dummyToken != t) {
			dummyToken->kind = t->kind;
			dummyToken->pos = t->pos;
			dummyToken->col = t->col;
			dummyToken->line = t->line;
			dummyToken->next = NULL;
			coco_string_delete(dummyToken->val);
			dummyToken->val = coco_string_create(t->val);
			t = dummyToken;
		}
		la = t;
	}
}

void Parser::Expect(int n) {
	if (la->kind==n) Get(); else { SynErr(n); }
}

void Parser::ExpectWeak(int n, int follow) {
	if (la->kind == n) Get();
	else {
		SynErr(n);
		while (!StartOf(follow)) Get();
	}
}

bool Parser::WeakSeparator(int n, int syFol, int repFol) {
	if (la->kind == n) {Get(); return true;}
	else if (StartOf(repFol)) {return false;}
	else {
		SynErr(n);
		while (!(StartOf(syFol) || StartOf(repFol) || StartOf(0))) {
			Get();
		}
		return StartOf(syFol);
	}
}

void Parser::Program() {
		using namespace std::chrono; 
		time_point<steady_clock> start = steady_clock::now(); 
		Statements();
		cmp_time = duration<double, milliseconds::period>(steady_clock::now() - start).count(); 
		start = steady_clock::now(); 
		prep(); 
		run(); 
		
		run_time = duration<double, milliseconds::period>(steady_clock::now() - start).count(); 
}

void Parser::Statements() {
		while (StartOf(1)) {
			Statement();
		}
}

void Parser::Statement() {
		if (is_declaration()) {
			VariableDeclaration();
			Expect(15 /* ";" */);
		} else if (StartOf(2)) {
			Expression();
			pop_val(); 
			while (la->kind == 16 /* "," */) {
				Get();
				Expression();
				pop_val(); 
			}
			Expect(15 /* ";" */);
		} else if (la->kind == 48 /* "Print" */ || la->kind == 49 /* "Println" */) {
			ProcCall();
			Expect(15 /* ";" */);
		} else if (la->kind == 19 /* "if" */) {
			Conditional();
		} else if (la->kind == 24 /* "while" */ || la->kind == 25 /* "for" */) {
			Loop();
		} else if (la->kind == 17 /* "{" */) {
			Scope();
		} else SynErr(51);
}

void Parser::VariableDeclaration() {
		Type type; 
		if (la->kind == _BOOL) {
			Get();
			type = bool_t;   
		} else if (la->kind == _INT) {
			Get();
			type = int_t;    
		} else if (la->kind == _FLOAT) {
			Get();
			type = float_t; 	
		} else SynErr(52);
		SingleDeclaration(type);
		while (la->kind == 16 /* "," */) {
			Get();
			SingleDeclaration(type);
		}
}

void Parser::Expression() {
		if (is_assignment()) {
			Assignment();
		} else if (StartOf(2)) {
			Expr();
		} else SynErr(53);
}

void Parser::ProcCall() {
		DefProcCall();
}

void Parser::Conditional() {
		If();
}

void Parser::Loop() {
		if (la->kind == 24 /* "while" */) {
			While();
		} else if (la->kind == 25 /* "for" */) {
			For();
		} else SynErr(54);
}

void Parser::Scope() {
		Expect(17 /* "{" */);
		Statements();
		Expect(18 /* "}" */);
}

void Parser::SingleDeclaration(Type type) {
		Expect(_ID);
		pointert ptr = choose_ident(t->val, type); 
		if (la->kind == _EQUALSIGN) {
			Get();
			Expression();
		} else if (la->kind == 15 /* ";" */ || la->kind == 16 /* "," */) {
			switch(type) {
			case bool_t: {
				add_bool(false);
			} break;
			case int_t: {
				add_int(0);
			} break;
			case float_t: {
				add_float(0);
			} break;
			}
			
		} else SynErr(55);
		switch(type) {
		case bool_t: {
			switch(types.back()) {
				case bool_t: {
					add_op(bool_bool_asgn_op, ptr, get_val());
				} break;
				case int_t: {
					cout << "asgn" << endl;
					exit(1);
				} break;
				case float_t: {
					cout << "asgn" << endl;
					exit(1);
				} break;
			}
		} break;
		case int_t: {
			switch(types.back()) {
				case bool_t: {
					cout << "asgn" << endl;
					exit(1);
				} break;
				case int_t: {
					add_op(int_int_asgn_op, ptr, get_val());
				} break;
				case float_t: {
					add_op(int_float_asgn_op, ptr, get_val());
				} break;
			}
		} break;
		case float_t: {
			switch(types.back()) {
				case bool_t: {
					cout << "asgn" << endl;
					exit(1);
				} break;
				case int_t: {
					add_op(float_int_asgn_op, ptr, get_val());
				} break;
				case float_t: {
					add_op(float_float_asgn_op, ptr, get_val());
				} break;
			}
		} break;
		}
		pop_val();
		
}

void Parser::If() {
		Expect(19 /* "if" */);
		Expect(20 /* "(" */);
		Expression();
		Expect(21 /* ")" */);
		if(types.back() != bool_t) {
		cout << "not bool in if" << endl;
		exit(1);
		}
		
		pointert if_pos = size(operations);
		add_op(if_op, -1, {unused.back(), -1});
		
		vector<pointert> else_pos;
		
		Scope();
		operations[if_pos].ptr = size(operations); 
		while (la->kind == 22 /* "elif" */) {
			else_pos.push_back(size(operations));
			add_op(else_op, -1, get_val());  
			
			Get();
			Expect(20 /* "(" */);
			Expression();
			Expect(21 /* ")" */);
			if(types.back() != bool_t) {
			cout << "not bool in elif" << endl;
			exit(1);
			}
			
			pointert if_pos = size(operations);
			add_op(if_op, -1, {unused.back(), -1});
			
			Scope();
			operations[if_pos].ptr = size(operations); 
		}
		if (la->kind == 23 /* "else" */) {
			else_pos.push_back(size(operations));
			add_op(else_op, -1, get_val());  
			
			Get();
			Scope();
		} else if (StartOf(3)) {
			pop_val(); 
		} else SynErr(56);
		for(pointert pos: else_pos) {
		operations[pos].ptr = size(operations);
		}
		
}

void Parser::While() {
		pointert begin = size(operations); 
		Expect(24 /* "while" */);
		Expect(20 /* "(" */);
		Expression();
		Expect(21 /* ")" */);
		if(types.back() != bool_t) {
		cout << "not bool in while" << endl;
		exit(1);
		}
		
		pointert pos = size(operations);
		add_op(while_op, -2, get_val()); 
		
		Scope();
		add_op(jump_op, begin);
		operations[pos].ptr = size(operations);
		
}

void Parser::For() {
		Expect(25 /* "for" */);
		Expect(20 /* "(" */);
		Expression();
		Expect(15 /* ";" */);
		Expression();
		Expect(15 /* ";" */);
		Expression();
		Expect(21 /* ")" */);
		Scope();
}

void Parser::Assignment() {
		Expect(_ID);
		auto [ptr, ltype] = get_ident(t->val); 
		switch (la->kind) {
		case _EQUALSIGN: {
			Get();
			Expression();
			Type rtype = pop_type();
			switch(ltype) {
			case bool_t: {
			add_type(bool_t);
			switch(rtype) {
				case bool_t: {
					add_op(bool_bool_asgn_op, ptr, get_val());
				} break;
				case int_t: {
					cout << "asgn" << endl;
					exit(1);
				} break;
				case float_t: {
					cout << "asgn" << endl;
					exit(1);
				} break;
			}
			} break;
			case int_t: {
			add_type(int_t);
			switch(rtype) {
				case bool_t: {
					cout << "asgn" << endl;
					exit(1);
				} break;
				case int_t: {
					add_op(int_int_asgn_op, ptr, get_val());
				} break;
				case float_t: {
					add_op(int_float_asgn_op, ptr, get_val());
				} break;
			}
			} break;
			case float_t: {
			add_type(float_t);
			switch(rtype) {
				case bool_t: {
					cout << "asgn" << endl;
					exit(1);
				} break;
				case int_t: {
					add_op(float_int_asgn_op, ptr, get_val());
				} break;
				case float_t: {
					add_op(float_float_asgn_op, ptr, get_val());
				} break;
			}
			} break;
			}
			
			break;
		}
		case _ADD_EQUALSIGN: {
			Get();
			Expression();
			Type rtype = pop_type();
			switch(ltype) {
			case bool_t:
			cout << "can not add assign to bool" << endl;
			exit(1);
			break;
			case int_t:
			add_type(int_t);
			switch(rtype) {
			case bool_t:
				cout << "asgn" << endl;
				exit(1);
				break;
			case int_t:
				add_op(int_int_add_asgn_op, ptr, get_val());
				break;
			case float_t:
				add_op(int_float_add_asgn_op, ptr, get_val());
				break;
			}
			break;
			case float_t:
			add_type(float_t);
			switch(rtype) {
			case bool_t:
				cout << "asgn" << endl;
				exit(1);
				break;
			case int_t:
				add_op(float_int_add_asgn_op, ptr, get_val());
				break;
			case float_t:
				add_op(float_float_add_asgn_op, ptr, get_val());
				break;
			}
			break;
			}
			
			break;
		}
		case _SUB_EQUALSIGN: {
			Get();
			Expression();
			Type rtype = pop_type();
			switch(ltype) {
			case bool_t:
			cout << "can not sub assign to bool" << endl;
			exit(1);
			break;
			case int_t:
			add_type(int_t);
			switch(rtype) {
			case bool_t:
				cout << "asgn" << endl;
				exit(1);
				break;
			case int_t:
				add_op(int_int_sub_asgn_op, ptr, get_val());
				break;
			case float_t:
				add_op(int_float_sub_asgn_op, ptr, get_val());
				break;
			}
			break;
			case float_t:
			add_type(float_t);
			switch(rtype) {
			case bool_t:
				cout << "asgn" << endl;
				exit(1);
				break;
			case int_t:
				add_op(float_int_sub_asgn_op, ptr, get_val());
				break;
			case float_t:
				add_op(float_float_sub_asgn_op, ptr, get_val());
				break;
			}
			break;
			}
			
			break;
		}
		case _MUL_EQUALSIGN: {
			Get();
			Expression();
			Type rtype = pop_type();
			switch(ltype) {
			case bool_t:
			cout << "can not mul assign to bool" << endl;
			exit(1);
			break;
			case int_t:
			add_type(int_t);
			switch(rtype) {
			case bool_t:
				cout << "asgn" << endl;
				exit(1);
				break;
			case int_t:
				add_op(int_int_mul_asgn_op, ptr, get_val());
				break;
			case float_t:
				add_op(int_float_mul_asgn_op, ptr, get_val());
				break;
			}
			break;
			case float_t:
			add_type(float_t);
			switch(rtype) {
			case bool_t:
				cout << "asgn" << endl;
				exit(1);
				break;
			case int_t:
				add_op(float_int_mul_asgn_op, ptr, get_val());
				break;
			case float_t:
				add_op(float_float_mul_asgn_op, ptr, get_val());
				break;
			}
			break;
			}
			
			break;
		}
		case _DIV_EQUALSIGN: {
			Get();
			Expression();
			Type rtype = pop_type();
			switch(ltype) {
			case bool_t:
			cout << "can not div assign to bool" << endl;
			exit(1);
			break;
			case int_t:
			add_type(int_t);
			switch(rtype) {
			case bool_t:
				cout << "asgn" << endl;
				exit(1);
				break;
			case int_t:
				add_op(int_int_div_asgn_op, ptr, get_val());
				break;
			case float_t:
				add_op(int_float_div_asgn_op, ptr, get_val());
				break;
			}
			break;
			case float_t:
			add_type(float_t);
			switch(rtype) {
			case bool_t:
				cout << "asgn" << endl;
				exit(1);
				break;
			case int_t:
				add_op(float_int_div_asgn_op, ptr, get_val());
				break;
			case float_t:
				add_op(float_float_div_asgn_op, ptr, get_val());
				break;
			}
			break;
			}
			
			break;
		}
		case _REM_EQUALSIGN: {
			Get();
			Expression();
			Type rtype = pop_type();
			if(ltype != int_t or rtype != int_t) {
			cout << "can not do rem assign with no int" << endl;
			exit(1);
			}
			add_type(int_t);
			add_op(int_int_rem_asgn_op, ptr, get_val());
			
			break;
		}
		default: SynErr(57); break;
		}
}

void Parser::Expr() {
		BoolTerm();
		while (la->kind == 26 /* "or" */) {
			Get();
			BoolTerm();
			if(types.back() != bool_t) {
			cout << "can not do or with no bool" << endl;
			exit(1);
			}
			types.pop_back();
			if(types.back() != bool_t) {
			cout << "can not do or with no bool" << endl;
			exit(1);
			}
			
			add_op(or_op, -1, get_vals()); 
			
		}
}

void Parser::BoolTerm() {
		BitOr();
		while (la->kind == 27 /* "and" */) {
			Get();
			BitOr();
			if(types.back() != bool_t) {
			cout << "can not do and with no bool" << endl;
			exit(1);
			}
			types.pop_back();
			if(types.back() != bool_t) {
			cout << "can not do and with no bool" << endl;
			exit(1);
			}
			
			add_op(and_op, -1, get_vals()); 
			
		}
}

void Parser::BitOr() {
		BitXor();
		while (la->kind == 28 /* "|" */) {
			Get();
			BitXor();
			if(pop_type() != int_t) {
			cout << "can not do bit or with no int" << endl;
			exit(1);
			}
			if(types.back() != int_t) {
			cout << "can not do bit or with no int" << endl;
			exit(1);
			}
			
			add_op(int_int_bit_or_op, -1, get_vals()); 
			
		}
}

void Parser::BitXor() {
		BitAnd();
		while (la->kind == 29 /* "^" */) {
			Get();
			BitAnd();
			if(pop_type() != int_t) {
			cout << "can not do bit xor with no int" << endl;
			exit(1);
			}
			if(types.back() != int_t) {
			cout << "can not do bit xor with no int" << endl;
			exit(1);
			}
			
			add_op(int_int_bit_xor_op, -1, get_vals()); 
			
		}
}

void Parser::BitAnd() {
		BoolEq();
		while (la->kind == 30 /* "&" */) {
			Get();
			BoolEq();
			if(pop_type() != int_t) {
			cout << "can not do bit and with no int" << endl;
			exit(1);
			}
			if(types.back() != int_t) {
			cout << "can not do bit and with no int" << endl;
			exit(1);
			}
			
			add_op(int_int_bit_and_op, -1, get_vals()); 
			
		}
}

void Parser::BoolEq() {
		BoolComp();
		if (la->kind == 31 /* "==" */ || la->kind == 32 /* "!=" */) {
			Type type = pop_type();
			if(type != types.back()) {
			cout << "can not do eq/uneq with different types" << endl;
			exit(1);
			}
			switch_type(bool_t);
			
			if (la->kind == 31 /* "==" */) {
				Get();
				BoolComp();
				switch(type) {
				case bool_t:
				add_op(bool_bool_eq_op, -1, get_vals());
				break;
				case int_t:
				add_op(int_int_eq_op, -1, get_vals());
				break;
				case float_t:
				add_op(float_float_eq_op, -1, get_vals());
				break;
				}
				
			} else {
				Get();
				BoolComp();
				switch(type) {
				case bool_t:
				add_op(bool_bool_uneq_op, -1, get_vals());
				break;
				case int_t:
				add_op(int_int_uneq_op, -1, get_vals());
				break;
				case float_t:
				add_op(float_float_uneq_op, -1, get_vals());
				break;
				}
				
			}
		}
}

void Parser::BoolComp() {
		if (StartOf(4)) {
			ArExpr();
			if (StartOf(5)) {
				if (la->kind == 33 /* "<" */) {
					Get();
					ArExpr();
					switch(pop_type()) {
					case bool_t:
					cout << "comp" << endl;
					exit(1);
					case int_t:
					switch(types.back()) {
					case bool_t:
						cout << "comp" << endl;
						exit(1);
						break;
					case int_t:
						switch_type(bool_t);
						add_op(int_int_ls_op, -1, get_vals());
						break;
					case float_t:
						switch_type(bool_t);
						add_op(float_int_ls_op, -1, get_vals());
						break;
					}
					break;
					case float_t:
					switch(types.back()) {
					case bool_t:
						cout << "comp" << endl;
						exit(1);
						break;
					case int_t:
						switch_type(bool_t);
						add_op(int_float_ls_op, -1, get_vals());
						break;
					case float_t:
						switch_type(bool_t);
						add_op(float_float_ls_op, -1, get_vals());
						break;
					}
					break;
					}
					
				} else if (la->kind == 34 /* ">" */) {
					Get();
					ArExpr();
					switch(pop_type()) {
					case bool_t:
					cout << "comp" << endl;
					exit(1);
					case int_t:
					switch(types.back()) {
					case bool_t:
						cout << "comp" << endl;
						exit(1);
						break;
					case int_t:
						switch_type(bool_t);
						add_op(int_int_gr_op, -1, get_vals());
						break;
					case float_t:
						switch_type(bool_t);
						add_op(float_int_gr_op, -1, get_vals());
						break;
					}
					break;
					case float_t:
					switch(types.back()) {
					case bool_t:
						cout << "comp" << endl;
						exit(1);
						break;
					case int_t:
						switch_type(bool_t);
						add_op(int_float_gr_op, -1, get_vals());
						break;
					case float_t:
						switch_type(bool_t);
						add_op(float_float_gr_op, -1, get_vals());
						break;
					}
					break;
					}
					
				} else if (la->kind == 35 /* "<=" */) {
					Get();
					ArExpr();
					switch(pop_type()) {
					case bool_t:
					cout << "comp" << endl;
					exit(1);
					case int_t:
					switch(types.back()) {
					case bool_t:
						cout << "comp" << endl;
						exit(1);
						break;
					case int_t:
						switch_type(bool_t);
						add_op(int_int_lseq_op, -1, get_vals());
						break;
					case float_t:
						switch_type(bool_t);
						add_op(float_int_lseq_op, -1, get_vals());
						break;
					}
					break;
					case float_t:
					switch(types.back()) {
					case bool_t:
						cout << "comp" << endl;
						exit(1);
						break;
					case int_t:
						switch_type(bool_t);
						add_op(int_float_lseq_op, -1, get_vals());
						break;
					case float_t:
						switch_type(bool_t);
						add_op(float_float_lseq_op, -1, get_vals());
						break;
					}
					break;
					}
					
				} else {
					Get();
					ArExpr();
					switch(pop_type()) {
					case bool_t:
					cout << "comp" << endl;
					exit(1);
					case int_t:
					switch(types.back()) {
					case bool_t:
						cout << "comp" << endl;
						exit(1);
						break;
					case int_t:
						switch_type(bool_t);
						add_op(int_int_greq_op, -1, get_vals());
						break;
					case float_t:
						switch_type(bool_t);
						add_op(float_int_greq_op, -1, get_vals());
						break;
					}
					break;
					case float_t:
					switch(types.back()) {
					case bool_t:
						cout << "comp" << endl;
						exit(1);
						break;
					case int_t:
						switch_type(bool_t);
						add_op(int_float_greq_op, -1, get_vals());
						break;
					case float_t:
						switch_type(bool_t);
						add_op(float_float_greq_op, -1, get_vals());
						break;
					}
					break;
					}
					
				}
			}
		} else if (la->kind == 37 /* "not" */) {
			Get();
			Expect(20 /* "(" */);
			Expression();
			Expect(21 /* ")" */);
			if(types.back() != bool_t) {
			cout << "can not do not with no bool" << endl;
			exit(1);
			}
			
			add_op(not_op, -1, get_val()); 
			
		} else SynErr(58);
}

void Parser::ArExpr() {
		if (la->kind == 38 /* "+" */) {
			Get();
		}
		Term();
		while (la->kind == 38 /* "+" */ || la->kind == 39 /* "-" */) {
			if (la->kind == 38 /* "+" */) {
				Get();
				Term();
				switch(pop_type()) {
				case bool_t:
				cout << "add" << endl;
				exit(1);
				break;
				case int_t:
				switch(types.back()) {
				case bool_t:
					cout << "add" << endl;
					exit(1);
					break;
				case int_t:
					add_op(int_int_add_op, -1, get_vals());
					break;
				case float_t:
					add_op(float_int_add_op, -1, get_vals());
					break;
				}
				break;
				case float_t:
				switch(types.back()) {
				case bool_t:
					cout << "add" << endl;
					exit(1);
					break;
				case int_t:
					switch_type(float_t);
					add_op(int_float_add_op, -1, get_vals());
					break;
				case float_t:
					add_op(float_float_add_op, -1, get_vals());
					break;
				}
				break;
				}
				
			} else {
				Get();
				Term();
				switch(pop_type()) {
				case bool_t:
				cout << "sub" << endl;
				exit(1);
				break;
				case int_t:
				switch(types.back()) {
				case bool_t:
					cout << "sub" << endl;
					exit(1);
					break;
				case int_t:
					add_op(int_int_sub_op, -1, get_vals());
					break;
				case float_t:
					add_op(float_int_sub_op, -1, get_vals());
					break;
				}
				break;
				case float_t:
				switch(types.back()) {
				case bool_t:
					cout << "sub" << endl;
					exit(1);
					break;
				case int_t:
					switch_type(float_t);
					add_op(int_float_sub_op, -1, get_vals());
					break;
				case float_t:
					add_op(float_float_sub_op, -1, get_vals());
					break;
				}
				break;
				}
				
			}
		}
}

void Parser::Term() {
		Value();
		while (la->kind == 40 /* "*" */ || la->kind == 41 /* "/" */ || la->kind == 42 /* "%" */) {
			if (la->kind == 40 /* "*" */) {
				Get();
				Value();
				switch(pop_type()) {
				case bool_t:
				cout << "mul" << endl;
				exit(1);
				break;
				case int_t:
				switch(types.back()) {
				case bool_t:
					cout << "mul" << endl;
					exit(1);
					break;
				case int_t:
					add_op(int_int_mul_op, -1, get_vals());
					break;
				case float_t:
					add_op(float_int_mul_op, -1, get_vals());
					break;
				}
				break;
				case float_t:
				switch(types.back()) {
				case bool_t:
					cout << "mul" << endl;
					exit(1);
					break;
				case int_t:
					switch_type(float_t);
					add_op(int_float_mul_op, -1, get_vals());
					break;
				case float_t:
					add_op(float_float_mul_op, -1, get_vals());
					break;
				}
				break;
				}
				
			} else if (la->kind == 41 /* "/" */) {
				Get();
				Value();
				switch(pop_type()) {
				case bool_t:
				cout << "div" << endl;
				exit(1);
				break;
				case int_t:
				switch(types.back()) {
				case bool_t:
					cout << "div" << endl;
					exit(1);
					break;
				case int_t:
					add_op(int_int_div_op, -1, get_vals());
					break;
				case float_t:
					add_op(float_int_div_op, -1, get_vals());
					break;
				}
				break;
				case float_t:
				switch(types.back()) {
				case bool_t:
					cout << "div" << endl;
					exit(1);
					break;
				case int_t:
					switch_type(float_t);
					add_op(int_float_div_op, -1, get_vals());
					break;
				case float_t:
					add_op(float_float_div_op, -1, get_vals());
					break;
				}
				break;
				}
				
			} else {
				Get();
				Value();
				if(pop_type() != int_t) {
				cout << "rem" << endl;
				exit(1);
				}
				if(types.back() != int_t) {
				cout << "rem" << endl;
				exit(1);
				}
				add_op(int_int_rem_op, -1, get_vals());
				
			}
		}
}

void Parser::Value() {
		if (la->kind == 20 /* "(" */) {
			Get();
			Expression();
			Expect(21 /* ")" */);
		} else if (StartOf(6)) {
			Boolean();
		} else if (la->kind == _NUMBER || la->kind == 39 /* "-" */) {
			Number();
		} else if (la->kind == _BOOL || la->kind == _INT || la->kind == _FLOAT) {
			FuncCall();
		} else if (la->kind == _ID) {
			Variable();
		} else SynErr(59);
}

void Parser::Boolean() {
		if (la->kind == 44 /* "true" */ || la->kind == 45 /* "True" */) {
			if (la->kind == 44 /* "true" */) {
				Get();
			} else {
				Get();
			}
			add_bool(true); 
		} else if (la->kind == 46 /* "false" */ || la->kind == 47 /* "False" */) {
			if (la->kind == 46 /* "false" */) {
				Get();
			} else {
				Get();
			}
			add_bool(false); 
		} else SynErr(60);
}

void Parser::Number() {
		wstring res; 
		if (la->kind == 39 /* "-" */) {
			Get();
			res += L'-'; 
		}
		Expect(_NUMBER);
		res += wstring(t->val); 
		if (la->kind == 43 /* "." */) {
			Get();
			res += L'.'; 
			Expect(_NUMBER);
			res += wstring(t->val); 
			add_float(stof(res)); 
		} else if (StartOf(7)) {
			add_int(stoi(res)); 
		} else SynErr(61);
}

void Parser::FuncCall() {
		DefFuncCall();
}

void Parser::Variable() {
		Expect(_ID);
		add_var(get_ident(t->val)); 
}

void Parser::DefFuncCall() {
		if (la->kind == _BOOL) {
			Get();
			Expect(20 /* "(" */);
			Expression();
			Expect(21 /* ")" */);
			switch(types.back()) {
			case int_t:
			switch_type(bool_t);
			add_op(int_to_bool_op, -1, get_val()); 
			break;
			case float_t:
			switch_type(bool_t);
			add_op(float_to_bool_op, -1, get_val()); 
			break;
			}
			
		} else if (la->kind == _INT) {
			Get();
			Expect(20 /* "(" */);
			Expression();
			Expect(21 /* ")" */);
			switch(types.back()) {
			case bool_t:
			switch_type(int_t);
			add_op(bool_to_int_op, -1, get_val()); 
			break;
			case float_t:
			switch_type(int_t);
			add_op(float_to_int_op, -1, get_val()); 
			break;
			}
			
		} else if (la->kind == _FLOAT) {
			Get();
			Expect(20 /* "(" */);
			Expression();
			Expect(21 /* ")" */);
			switch(types.back()) {
			case bool_t:
			switch_type(float_t);
			add_op(bool_to_float_op, -1, get_val()); 
			break;
			case int_t:
			switch_type(float_t);
			add_op(int_to_float_op, -1, get_val()); 
			break;
			}
			
		} else SynErr(62);
}

void Parser::DefProcCall() {
		if (la->kind == 48 /* "Print" */) {
			Print();
		} else if (la->kind == 49 /* "Println" */) {
			Println();
		} else SynErr(63);
}

void Parser::Print() {
		Expect(48 /* "Print" */);
		Expect(20 /* "(" */);
		Expression();
		switch(pop_type()) {
		case bool_t:
		add_op(bool_print_op, -2, get_val());
		break;
		case int_t:
		add_op(int_print_op, -2, get_val());
		break;
		case float_t:
		add_op(float_print_op, -2, get_val());
		break;
		}
		
		while (la->kind == 16 /* "," */) {
			add_op(print_del_op); 
			Get();
			Expression();
			switch(pop_type()) {
			case bool_t:
			add_op(bool_print_op, -2, get_val());
			break;
			case int_t:
			add_op(int_print_op, -2, get_val());
			break;
			case float_t:
			add_op(float_print_op, -2, get_val());
			break;
			}
			
		}
		Expect(21 /* ")" */);
}

void Parser::Println() {
		Expect(49 /* "Println" */);
		Expect(20 /* "(" */);
		Expression();
		switch(pop_type()) {
		case bool_t:
		add_op(bool_print_op, -2, get_val());
		break;
		case int_t:
		add_op(int_print_op, -2, get_val());
		break;
		case float_t:
		add_op(float_print_op, -2, get_val());
		break;
		}
		
		while (la->kind == 16 /* "," */) {
			add_op(print_del_op); 
			Get();
			Expression();
			switch(pop_type()) {
			case bool_t:
			add_op(bool_print_op, -2, get_val());
			break;
			case int_t:
			add_op(int_print_op, -2, get_val());
			break;
			case float_t:
			add_op(float_print_op, -2, get_val());
			break;
			}
			
		}
		Expect(21 /* ")" */);
		add_op(print_endl_op); 
}




// If the user declared a method Init and a mehtod Destroy they should
// be called in the contructur and the destructor respctively.
//
// The following templates are used to recognize if the user declared
// the methods Init and Destroy.

template<typename T>
struct ParserInitExistsRecognizer {
	template<typename U, void (U::*)() = &U::Init>
	struct ExistsIfInitIsDefinedMarker{};

	struct InitIsMissingType {
		char dummy1;
	};
	
	struct InitExistsType {
		char dummy1; char dummy2;
	};

	// exists always
	template<typename U>
	static InitIsMissingType is_here(...);

	// exist only if ExistsIfInitIsDefinedMarker is defined
	template<typename U>
	static InitExistsType is_here(ExistsIfInitIsDefinedMarker<U>*);

	enum { InitExists = (sizeof(is_here<T>(NULL)) == sizeof(InitExistsType)) };
};

template<typename T>
struct ParserDestroyExistsRecognizer {
	template<typename U, void (U::*)() = &U::Destroy>
	struct ExistsIfDestroyIsDefinedMarker{};

	struct DestroyIsMissingType {
		char dummy1;
	};
	
	struct DestroyExistsType {
		char dummy1; char dummy2;
	};

	// exists always
	template<typename U>
	static DestroyIsMissingType is_here(...);

	// exist only if ExistsIfDestroyIsDefinedMarker is defined
	template<typename U>
	static DestroyExistsType is_here(ExistsIfDestroyIsDefinedMarker<U>*);

	enum { DestroyExists = (sizeof(is_here<T>(NULL)) == sizeof(DestroyExistsType)) };
};

// The folloing templates are used to call the Init and Destroy methods if they exist.

// Generic case of the ParserInitCaller, gets used if the Init method is missing
template<typename T, bool = ParserInitExistsRecognizer<T>::InitExists>
struct ParserInitCaller {
	static void CallInit(T *t) {
		// nothing to do
	}
};

// True case of the ParserInitCaller, gets used if the Init method exists
template<typename T>
struct ParserInitCaller<T, true> {
	static void CallInit(T *t) {
		t->Init();
	}
};

// Generic case of the ParserDestroyCaller, gets used if the Destroy method is missing
template<typename T, bool = ParserDestroyExistsRecognizer<T>::DestroyExists>
struct ParserDestroyCaller {
	static void CallDestroy(T *t) {
		// nothing to do
	}
};

// True case of the ParserDestroyCaller, gets used if the Destroy method exists
template<typename T>
struct ParserDestroyCaller<T, true> {
	static void CallDestroy(T *t) {
		t->Destroy();
	}
};

void Parser::Parse() {
	t = NULL;
	la = dummyToken = new Token();
	la->val = coco_string_create(L"Dummy Token");
	Get();
	Program();
	Expect(0);
}

Parser::Parser(Scanner *scanner) {
	maxT = 50;

	ParserInitCaller<Parser>::CallInit(this);
	dummyToken = NULL;
	t = la = NULL;
	minErrDist = 2;
	errDist = minErrDist;
	this->scanner = scanner;
	errors = new Errors();
}

bool Parser::StartOf(int s) {
	const bool T = true;
	const bool x = false;

	static bool set[8][52] = {
		{T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x},
		{x,T,T,x, x,x,x,x, x,x,x,x, T,T,T,x, x,T,x,T, T,x,x,x, T,T,x,x, x,x,x,x, x,x,x,x, x,T,T,T, x,x,x,x, T,T,T,T, T,T,x,x},
		{x,T,T,x, x,x,x,x, x,x,x,x, T,T,T,x, x,x,x,x, T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,T,T,T, x,x,x,x, T,T,T,T, x,x,x,x},
		{T,T,T,x, x,x,x,x, x,x,x,x, T,T,T,x, x,T,T,T, T,x,x,x, T,T,x,x, x,x,x,x, x,x,x,x, x,T,T,T, x,x,x,x, T,T,T,T, T,T,x,x},
		{x,T,T,x, x,x,x,x, x,x,x,x, T,T,T,x, x,x,x,x, T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, x,x,x,x, T,T,T,T, x,x,x,x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,T,T,T, T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, T,T,T,T, x,x,x,x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,T, T,x,x,x, x,T,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,x,T,T, T,T,T,x, x,x,x,x, x,x,x,x}
	};



	return set[s][la->kind];
}

Parser::~Parser() {
	ParserDestroyCaller<Parser>::CallDestroy(this);
	delete errors;
	delete dummyToken;
}

Errors::Errors() {
	count = 0;
}

void Errors::SynErr(int line, int col, int n) {
	wchar_t* s;
	switch (n) {
			case 0: s = coco_string_create(L"EOF expected"); break;
			case 1: s = coco_string_create(L"ID expected"); break;
			case 2: s = coco_string_create(L"NUMBER expected"); break;
			case 3: s = coco_string_create(L"EQUALSIGN expected"); break;
			case 4: s = coco_string_create(L"ADD_EQUALSIGN expected"); break;
			case 5: s = coco_string_create(L"SUB_EQUALSIGN expected"); break;
			case 6: s = coco_string_create(L"MUL_EQUALSIGN expected"); break;
			case 7: s = coco_string_create(L"DIV_EQUALSIGN expected"); break;
			case 8: s = coco_string_create(L"REM_EQUALSIGN expected"); break;
			case 9: s = coco_string_create(L"ENDL expected"); break;
			case 10: s = coco_string_create(L"DEL expected"); break;
			case 11: s = coco_string_create(L"VAR expected"); break;
			case 12: s = coco_string_create(L"BOOL expected"); break;
			case 13: s = coco_string_create(L"INT expected"); break;
			case 14: s = coco_string_create(L"FLOAT expected"); break;
			case 15: s = coco_string_create(L"\";\" expected"); break;
			case 16: s = coco_string_create(L"\",\" expected"); break;
			case 17: s = coco_string_create(L"\"{\" expected"); break;
			case 18: s = coco_string_create(L"\"}\" expected"); break;
			case 19: s = coco_string_create(L"\"if\" expected"); break;
			case 20: s = coco_string_create(L"\"(\" expected"); break;
			case 21: s = coco_string_create(L"\")\" expected"); break;
			case 22: s = coco_string_create(L"\"elif\" expected"); break;
			case 23: s = coco_string_create(L"\"else\" expected"); break;
			case 24: s = coco_string_create(L"\"while\" expected"); break;
			case 25: s = coco_string_create(L"\"for\" expected"); break;
			case 26: s = coco_string_create(L"\"or\" expected"); break;
			case 27: s = coco_string_create(L"\"and\" expected"); break;
			case 28: s = coco_string_create(L"\"|\" expected"); break;
			case 29: s = coco_string_create(L"\"^\" expected"); break;
			case 30: s = coco_string_create(L"\"&\" expected"); break;
			case 31: s = coco_string_create(L"\"==\" expected"); break;
			case 32: s = coco_string_create(L"\"!=\" expected"); break;
			case 33: s = coco_string_create(L"\"<\" expected"); break;
			case 34: s = coco_string_create(L"\">\" expected"); break;
			case 35: s = coco_string_create(L"\"<=\" expected"); break;
			case 36: s = coco_string_create(L"\">=\" expected"); break;
			case 37: s = coco_string_create(L"\"not\" expected"); break;
			case 38: s = coco_string_create(L"\"+\" expected"); break;
			case 39: s = coco_string_create(L"\"-\" expected"); break;
			case 40: s = coco_string_create(L"\"*\" expected"); break;
			case 41: s = coco_string_create(L"\"/\" expected"); break;
			case 42: s = coco_string_create(L"\"%\" expected"); break;
			case 43: s = coco_string_create(L"\".\" expected"); break;
			case 44: s = coco_string_create(L"\"true\" expected"); break;
			case 45: s = coco_string_create(L"\"True\" expected"); break;
			case 46: s = coco_string_create(L"\"false\" expected"); break;
			case 47: s = coco_string_create(L"\"False\" expected"); break;
			case 48: s = coco_string_create(L"\"Print\" expected"); break;
			case 49: s = coco_string_create(L"\"Println\" expected"); break;
			case 50: s = coco_string_create(L"??? expected"); break;
			case 51: s = coco_string_create(L"invalid Statement"); break;
			case 52: s = coco_string_create(L"invalid VariableDeclaration"); break;
			case 53: s = coco_string_create(L"invalid Expression"); break;
			case 54: s = coco_string_create(L"invalid Loop"); break;
			case 55: s = coco_string_create(L"invalid SingleDeclaration"); break;
			case 56: s = coco_string_create(L"invalid If"); break;
			case 57: s = coco_string_create(L"invalid Assignment"); break;
			case 58: s = coco_string_create(L"invalid BoolComp"); break;
			case 59: s = coco_string_create(L"invalid Value"); break;
			case 60: s = coco_string_create(L"invalid Boolean"); break;
			case 61: s = coco_string_create(L"invalid Number"); break;
			case 62: s = coco_string_create(L"invalid DefFuncCall"); break;
			case 63: s = coco_string_create(L"invalid DefProcCall"); break;

		default:
		{
			wchar_t format[20];
			coco_swprintf(format, 20, L"error %d", n);
			s = coco_string_create(format);
		}
		break;
	}
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	coco_string_delete(s);
	count++;
}

void Errors::Error(int line, int col, const wchar_t *s) {
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	count++;
}

void Errors::Warning(int line, int col, const wchar_t *s) {
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
}

void Errors::Warning(const wchar_t *s) {
	wprintf(L"%ls\n", s);
}

void Errors::Exception(const wchar_t* s) {
	wprintf(L"%ls", s); 
	exit(1);
}


