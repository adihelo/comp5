#include "generator.hpp"
#include "hw3_output.hpp"
#include "parser.hpp"
#include "bp.hpp"
#include <string>
#include <sstream>
#include <iomanip>
using namespace output;
using std::string;

int curr_reg = 0;

string freshVar() {
    string reg = "%R";
    reg += to_string(curr_reg++);
    return reg;
}

string convert_to_llvm_type(const string& type){
    if(type == "void"){
        return "void";
    } else if (type == "bool"){
        return "i1";
    } else if (type == "byte"){
        return "i8";
    } else{
        return "i32";
    }
}

void llvmFuncDecl(const string& retType, const string& funcName, vector<string>& argTypes){

    string define_command = "define " + convert_to_llvm_type(retType) + " @" + funcName + "(";
    for (int i = 0; i < argTypes.size()-1 ; ++i) {
        define_command += convert_to_llvm_type(argTypes[i]) + ",";
    }
    define_command += convert_to_llvm_type(argTypes.back()) + ") {";
    buffer.emit(define_command);
}
