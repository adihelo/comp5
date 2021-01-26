#ifndef GENERATOR_H
#define GENERATOR_H

#include "bp.hpp"
#include "hw3_output.hpp"
#include "parser.hpp"
#define buffer CodeBuffer::instance()


/********************* helper functions ******************/

string freshVar();
string convert_to_llvm_type(const string& type);
void emitComman(const string& command);
void zext(string& reg_to_zext, const string& type);
void FuncDeclAllocation(int argsNum);
void storeFuncArg(const string& type, int offset, int argsNumber);
void storeVariable(const string& var_name, const string& value, const string& type, int offset, const int& argsNum);
void addToFalseList(Exp* exp, pair<int,BranchLabelIndex> branch);//done
void addToTrueList(Exp* exp, pair<int,BranchLabelIndex> branch);
string phi(Exp* exp);




/******************** LLVM functions ********************/
void llvmFuncDecl(string retType, const string& funcName, vector<string>& argTypes);
void llvmExpRelOp(Exp* result, Exp* exp1, Exp* exp2, const string& binop);
void llvmExpBinOp(Exp* result, Exp* exp1, Exp* exp2, const string& relop, bool isByte);
void llvmIfStmt(Statement* statement, Exp* cond, Statement* inst, string label);
void llvmIfElseStmt(Statement* statement, Exp* cond, Statement* inst_true, Statement* inst_false, Statement* marker, string label_true, string label_fals&e);
void llvmWhileStmt(Statement* statement, Exp* cond, Statement* inst, string break_label, string inst_label);















#endif