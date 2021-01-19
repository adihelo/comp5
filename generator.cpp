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
    string reg = "%reg";
    reg += to_string(curr_reg++);
    return reg;
}

void emitComman(const string& command){
    buffer.emit(command);
}

void zext(string& reg_to_zext, const string& type){
    if (type == "i1" || type == "i8"){
        string zexted_reg = freshVar();
        string zext_command = "  " + zexted_reg + " = zext " + type + " " + reg_to_zext + " to i32";
        emitComman(zext_command);
        reg_to_zext = zexted_reg;
    }
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

void FuncDeclAllocation(int argsNum){
    buffer.emit("   %locals = alloca [50 x i32]");
    if (argsNum){
        buffer.emit("   %params = alloca [" + to_string(argsNum) + " x i32]");
    }
    curr_reg = argsNum +1;
}

void storeFuncArg(const string& type, const string& offset, int argsNumber){
    string variable = freshVar();
    buffer.emit("   " + variable + " = getelementptr [" + to_string(argsNumber) + " x i32, [" + to_string(argsNumber) + " x i32]* %params, i32 0, i32 " + offset);

}

void llvmFuncDecl(const string& retType, const string& funcName, vector<string>& argTypes){

    string define_command = "define " + convert_to_llvm_type(retType) + " @" + funcName + "(";
    for (int i = 0; i < argTypes.size()-1 ; ++i) {
        define_command += convert_to_llvm_type(argTypes[i]) + ",";
    }
    define_command += convert_to_llvm_type(argTypes.back()) + ") {";
    buffer.emit(define_command);
}
