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




/******************** LLVM functions ********************/
void llvmFuncDecl(string retType, const string& funcName, vector<string>& argTypes);
void llvmExpRelOp(Exp* result, Exp* exp1, Exp* exp2, const string& binop);
void llvmExpBinOp(Exp* result, Exp* exp1, Exp* exp2, const string& relop, bool isByte);















#endif