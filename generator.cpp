#include "generator.hpp"
#include "hw3_output.hpp"
//#include "parser.hpp"
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

void emitCommand(const string& command){
    buffer.emit(command);
}

void zext(string& reg_to_zext, const string& type){
    if (type == "i1" || type == "i8"){
        string zexted_reg = freshVar();
        string zext_command = "  " + zexted_reg + " = zext " + type + " " + reg_to_zext + " to i32";
        emitCommand(zext_command);
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

string llvmOpCommand(const string& operation){
    if (operation == "<")
        return "slt";
    else if (operation == "<=")
        return "sle";
    else if (operation == ">")
        return  "sgt";
    else if (operation == ">=")
        return  "sge";
    else if (operation == "==")
        return  "eq";
    else if (operation == "!=")
        return  "ne";
    else if (operation == "+")
        return  "add";
    else if (operation == "-")
        return  "sub";
    else if (operation == "*")
        return  "mul";
    else
        return  "sdiv";
}

void FuncDeclAllocation(int argsNum){
    buffer.emit("   %locals = alloca [50 x i32]");
    if (argsNum){
        buffer.emit("   %params = alloca [" + to_string(argsNum) + " x i32]");
    }
    curr_reg = argsNum +1;
}

void storeFuncArg(const string& type, int offset, int argsNumber){
    string variable = freshVar();
    buffer.emit("   " + variable + " = getelementptr [" + to_string(argsNumber) + " x i32, [" + to_string(argsNumber) + " x i32]* %params, i32 0, i32 " + to_string(offset));
    string off_reg ="%" + to_string(offset);
    zext(off_reg, convert_to_llvm_type(type));
    buffer.emit("   store i32 " + off_reg + ", i32* " + variable);

}

void storeVariable(string value, const string& type, int offset, const int& argsNum){

    zext(value,convert_to_llvm_type(type));
    if(value=="0") {
        value=freshVar();
        buffer.emit("   " + value + " = add " + convert_to_llvm_type(type)+" 0, 0");
    }
    string var_ptr = freshVar();
    if (offset >= 0) {
        buffer.emit("   " + var_ptr + " = getelementptr [50 x i32], [50 x i32]* %locals, i32 0, i32 " + to_string(offset) );
        buffer.emit("   store i32 " + value + ", i32* " + var_ptr);
    } else {
        offset++;
        buffer.emit("   " + var_ptr + " = getelementptr [" + to_string(argsNum)+ " x i32], [" + to_string(argsNum)+ " x i32]* %params, i32 0, i32 " + to_string(-offset));
        buffer.emit("   store i32 " + value + ", i32* " + var_ptr);
    }
}

string phi(Exp* exp){
    string bool_var = freshVar();
    string phi_label = "phi_result" + buffer.genLabel();
    string phi_false_label = "phi_false" + buffer.genLabel();
    string phi_true_label = "phi_true" + buffer.genLabel();

    // remove ":" from labels
    phi_label.pop_back();
    phi_false_label.pop_back();
    phi_true_label.pop_back();

    buffer.bpatch(exp->trueList, phi_true_label);
    buffer.bpatch(exp->falseList, phi_false_label);
    buffer.emit(phi_true_label + ":");
    buffer.emit("   br label %" + phi_label );
    buffer.emit(phi_false_label + ":");
    buffer.emit("   br label %" + phi_label );
    buffer.emit("   " + phi_label);
    buffer.emit("   "+ bool_var +" = phi i1 [ true, %" + phi_true_label +"], [ false, %" + phi_false_label +" ]");
    return bool_var;

}

void llvmFuncDecl(string retType, const string& funcName, vector<string>& argTypes){

    string define_command = "define " + convert_to_llvm_type(retType) + " @" + funcName + "(";
    for (int i = 0; i < argTypes.size()-1 ; ++i) {
        define_command += convert_to_llvm_type(argTypes[i]) + ",";
    }
    define_command += convert_to_llvm_type(argTypes.back()) + ") {";
    buffer.emit(define_command);
}

void llvmIfStmt(Statement* statement, Exp* cond, Statement* inst, string label){

    buffer.bpatch(cond->trueList, label);
    statement->nextlist = buffer.merge(cond->falseList, inst->nextlist);
    string if_end_label = buffer.genLabel();
    buffer.bpatch(statement->nextlist, if_end_label);

}

void llvmIfElseStmt(Statement* statement, Exp* cond, Statement* inst_true, Statement* inst_false, Statement* marker, string label_true, string label_false){

    buffer.bpatch(cond->trueList, label_true);
    buffer.bpatch(cond->falseList, label_false);
    statement->nextlist = buffer.merge(inst_true->nextlist, buffer.merge(marker->nextlist, inst_false->nextlist));
    statement->breaklist = buffer.merge(inst_true->breaklist, inst_false->breaklist);//NEW
    string if_else_end_label = buffer.genLabel();
    buffer.bpatch(statement->nextlist, if_else_end_label);
}

void llvmWhileStmt(Statement* statement, Exp* cond, Statement* inst, string break_label, string inst_label){

    buffer.bpatch(inst->nextlist, inst_label);
    buffer.bpatch(cond->trueList, break_label);
    statement->nextlist = buffer.merge(cond->falseList, inst->breaklist);
    buffer.emit("   br label %" + inst_label);
    string while_end_label = buffer.genLabel();
    buffer.bpatch(statement->nextlist, while_end_label);

}

void llvmExpRelOp(Exp* result, Exp* exp1, Exp* exp2, const string& binop){
    string reg=freshVar();
    buffer.emit("%" + reg + " = icmp "+binop+" i32 %"+exp1->reg+", %"+exp1->reg);
    int line= buffer.emit("br i1 %" + reg + ", label @, label @");
    addToFalseList(result, make_pair(line, SECOND));
    addToTrueList(result, make_pair(line, FIRST));

}

string llvmExpBinOp(Exp* result, Exp* exp1, Exp* exp2, const string& relop, bool isByte){
    string op=relop;
    string reg=freshVar();
    if(relop == "sdiv"){ //div check whether exp2 is zero or not.
        buffer.emit(reg+" = icmp eq i32 0, "+exp2->reg);
        int line=buffer.emit("br i1 %" + reg + ", label @, label @");
        buffer.bpatch(buffer.makelist(make_pair(line,FIRST)),buffer.genLabel());
        buffer.emit("call void @print(i8* getelementptr ([23 x i8], [23 x i8]* @error, i64 0, i64 0))");
        buffer.emit("call void @exit(i32 0)");
        int end=buffer.emit("br label @");
        buffer.bpatch(buffer.merge(buffer.makelist(make_pair(line,SECOND)),buffer.makelist(make_pair(end,FIRST))),buffer.genLabel());
        if(isByte)  op="udiv";
        
    }
    reg=freshVar();
    buffer.emit(reg+" = "+op+" i32 "+exp1->reg+", "+exp2->reg);
     string new_reg;
    if (isByte) //on exp is byte
    {
        //trunc and zext ( HW page 4)
        string prev_reg=reg;
        new_reg=freshVar();
        buffer.emit(new_reg+" = trunc i32 "+prev_reg+" to i8");
		prev_reg=new_reg;
        new_reg=freshVar();
		buffer.emit(new_reg+"=zext i8 "+prev_reg+" to i32");

    }
    if(isByte) return new_reg;
    return reg;
}
/*
   emit function for call
*/
string call_emit(const string& func_type, const string& func_name, vector<pair<string,string>> var_vec){  //NEW
        string emit_str;

        string call_res_reg;
           if(func_name=="print"){
               int reg=lastStringReg-1;
               int size=lastStringSize;
                buffer.emit("call void @print(i8* getelementptr (["+to_string(size)+" x i8], ["+to_string(size)+" x i8]* @string"+to_string(reg)+", i64 0, i64 0))"); 
               
           }else{
                if(func_type=="VOID"){
                    emit_str="call void";
                }else{
                    call_res_reg = freshVar();
                    emit_str=call_res_reg+" = call i32";
               
                }
                emit_str+=" @"+func_name+"(";
                if(!var_vec.empty()){
                    for (int i=0; i<var_vec.size(); i++){
                        emit_str+="i32 "+var_vec[i].second;                 
                        if(i<var_vec.size()-1) emit_str+=" ,";   
                    }
                }else{
                   emit_str+=")";
                }
                buffer.emit(emit_str);
           }
        return call_res_reg;
    }
string emit_id(int offset)
	{
        string reg1=freshVar();
        buffer.emit(reg1+ " = getelementptr [50 x i32], [50 x i32]* %locals, i32 0, i32 " + to_string(offset));
        string reg2=freshVar();
        buffer.emit(reg2+" = load i32, i32* "+reg1);
        return reg2;
					
	}  
void addToFalseList(Exp* exp, pair<int,BranchLabelIndex> branch){
    exp->falseList=CodeBuffer::merge(exp->falseList,CodeBuffer::makelist(branch));


}
void addToTrueList(Exp* exp, pair<int,BranchLabelIndex> branch){
    exp->trueList=CodeBuffer::merge(exp->trueList,CodeBuffer::makelist(branch));

}

string llvmExpIsBool(Exp* exp){
    string true_label=  buffer.genLabel();
    buffer.bpatch(exp->trueList, true_label); 
    exp->trueList.clear();
    int line =buffer.emit("br label @");
    addToTrueList(exp,make_pair(line,FIRST));
    string false_label=  buffer.genLabel();
    buffer.bpatch(exp->falseList, false_label); 
    int line2 =buffer.emit("br label @");
    addToTrueList(exp,make_pair(line2,FIRST));
    buffer.bpatch(exp->trueList, buffer.genLabel());
    string reg = freshVar();
    buffer.emit(reg+" = phi i32 [1, %"+true_label+"], [0, %"+false_label+"]"); 
    return reg;

}
