#ifndef GENERATOR_H
#define GENERATOR_H

#include "bp.hpp"
#include "hw3_output.hpp"
#include "parser.hpp"
#define buffer CodeBuffer::instance()

extern int lastStringSize;
extern int lastStringReg;

/********************* helper functions ******************/

string freshVar();
string convert_to_llvm_type(const string& type);
void emitCommand(const string& command);
void zext(string& reg_to_zext, const string& type);
void FuncDeclAllocation(int argsNum);
void storeFuncArg(const string& type, int offset, int argsNumber);
void storeVariable(string value, const string& type, int offset, const int& argsNum);
void addToFalseList(Exp* exp, pair<int,BranchLabelIndex> branch);//done
void addToTrueList(Exp* exp, pair<int,BranchLabelIndex> branch);
string phi(Exp* exp);

string call_emit(const string& func_type, const string& func_name, vector<pair<string,string>> var_vec);
string emit_id(int offset, int argsSize);
string llvmExpIsBool(Exp* exp);
/******************** LLVM functions ********************/
void llvmFuncDecl(string retType, const string& funcName, vector<string>& argTypes);
void llvmExpRelOp(Exp* result, Exp* exp1, Exp* exp2, const string& binop);
void llvmIfStmt(Statement* statement, Exp* cond, Statement* inst, string label);
void llvmIfElseStmt(Statement* statement, Exp* cond, Statement* inst_true, Statement* inst_false, Statement* marker, string label_true, string label_false);
void llvmWhileStmt(Statement* statement, Exp* cond, Statement* inst, string break_label, string inst_label);
string llvmExpBinOp(Exp* result, Exp* exp1, Exp* exp2, const string& relop, bool isByte);
string llvmOpCommand(const string& operation);
















#endif