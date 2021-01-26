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

void storeFuncArg(const string& type, const string& offset, int argsNumber){
    string variable = freshVar();
    buffer.emit("   " + variable + " = getelementptr [" + to_string(argsNumber) + " x i32, [" + to_string(argsNumber) + " x i32]* %params, i32 0, i32 " + offset);
    string off_reg ="%" + offset;
    zext(off_reg, convert_to_llvm_type(type));
    buffer.emit("   store i32 " + off_reg + ", i32* " + variable);

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

void llvmFuncDecl(const string& retType, const string& funcName, vector<string>& argTypes){

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
    addToFalseList(result, {line, SECOND});
    addToTrueList(result, {line, FIRST});

}

void llvmExpBinOp(Exp* result, Exp* exp1, Exp* exp2, const string& relop, bool isByte){
    string op=relop;
    string reg=freshVar();
    if(relop == "sdiv"){ //div check whether exp2 is zero or not.
        buffer.emit(reg+" = icmp eq i32 0, "+exp2->reg);
        int line=buffer.emit("br i1 %" + reg + ", label @, label @");
        buffer.bpatch(buffer.makelist({line,FIRST}),buffer.genLabel());
        buffer.emit("call void @print(i8* getelementptr ([23 x i8], [23 x i8]* @error, i64 0, i64 0))");
        buffer.emit("call void @exit(i32 0)");
        int end=buffer.emit("br label @");
        buffer.bpatch(buffer.merge(buffer.makelist({line,SECOND}),buffer.makelist({end,FIRST})),buffer.genLabel());
        if(isByte)  op="udiv";
        
    }
    
    buffer.emit(to_string(curr_reg)+" = "+op+" i32 "+exp1->reg+", "+exp2->reg);
    if (isByte) //on exp is byte
    {
        //truncating and zext ( HW page 4)
        string prev_reg=reg;
        string new_reg=freshVar();
		//buffer.emit("%"+new_reg+"=inttoptr i32 %"+to_string( prev_reg)+" to i8*");
		//prev_reg=new_reg;
        //new_reg=freshVar();
		//buffer.emit("%"+new_reg+"=ptrtoint i8* %"+to_string(prev_reg)+" to i8");
        
         buffer.emit(new_reg+" = trunc i32 "+prev_reg+" to i8*");

		prev_reg=new_reg;
        new_reg=freshVar();
		buffer.emit(new_reg+"=zext i8 "+prev_reg+" to i32");

    }
    
}
/*
   emit function for call
*/
void call_emit(const string& func_type, const string& func_name, vector<pair<string,int>> var_vec){  //NEW
        string emit_str;
         if (func_name=="print") { //print like descirbed in pdf.
           // string size=to_string(table.lastStringSize);
            //TODO: Add like described for exit in hw5 pdf file.
            buffer.emit(emit_str);
            return;
        }else{
            if(func_type=="VOID"){
                emit_str="call void";
            }else{
                emit_str=to_string(curr_reg)+" = call i32";
                curr_reg = freshVar();
            }
            emit_str+=" @"+func_name+"(";
            if(!var_vec.empty()){
                for (int i=0; i<var_vec.size()-1; i++){
                    emit_str+="i32 "+to_string(var_vec[i].second)+", ";
                }
                emit_str+="i32 "+to_string(var_vec.back().second)+")";
            }
            buffer.emit(emit_str);
        }
    }
    
void addToFalseList(Exp* exp, pair<int,BranchLabelIndex> branch){
    exp->falseList=CodeBuffer::merge(exp->falseList,CodeBuffer::makelist(branch));


}
void addToTrueList(Exp* exp, pair<int,BranchLabelIndex> branch){
    exp->trueList=CodeBuffer::merge(exp->trueList,CodeBuffer::makelist(branch));

}


