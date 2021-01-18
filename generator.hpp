#ifndef GENERATOR_H
#define GENERATOR_H

#include "bp.hpp"
#include "hw3_output.hpp"
#include "parser.hpp"
#define buffer CodeBuffer::instance()


/********************* helper functions ******************/

string freshVar();
string convert_to_llvm_type(const string& type);




/******************** LLVM functions ********************/

void llvmFuncDecl(string retType, const string& funcName, vector<string>& argTypes);















#endif