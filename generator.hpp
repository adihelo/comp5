#ifndef GENERATOR_H
#define GENERATOR_H

#include "bp.hpp"
#include "hw3_output.hpp"
#include "parser.hpp"
#define buffer CodeBuffer::instance()

int lastStringSize=0;
int lastStringReg=0;
/********************* helper functions ******************/

string freshVar();
string convert_to_llvm_type(const string& type);
void emitComman(const string& command);
void zext(string& reg_to_zext, const string& type);
void FuncDeclAllocation(int argsNum);
void storeFuncArg(const string& type, int offset, int argsNumber);
void addToFalseList(Exp* exp, pair<int,BranchLabelIndex> branch);//done
void addToTrueList(Exp* exp, pair<int,BranchLabelIndex> branch);

string call_emit(const string& func_type, const string& func_name, vector<pair<string,int>> var_vec);
string emit_id(int offset);

/******************** LLVM functions ********************/
void llvmFuncDecl(string retType, const string& funcName, vector<string>& argTypes);
void llvmExpRelOp(Exp* result, Exp* exp1, Exp* exp2, const string& binop);
string llvmExpBinOp(Exp* result, Exp* exp1, Exp* exp2, const string& relop, bool isByte);















#endif