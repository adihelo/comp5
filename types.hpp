#ifndef _TYPES_
#define _TYPES_
#include <string>
#include <vector>
#include <sstream>
#include "hw3_output.hpp"
using namespace std;
extern int yylineno;

class Node {
     public:
    string name;
    string type;
    virtual void makeItPolymorphic(){}
};


class Identifier: public Node {
public:
    explicit Identifier(string new_name){
        name = new_name;
    }
};

class Type: public Node{
public:
    explicit Type(string new_type) {
        type = new_type;
    }

};

class Exp: public Node{
public:
    explicit Exp(string type){
        this->type = type;
    }
};

class Num : public Node{
    int value;
public:
    explicit Num(string num){
        name = num;
        stringstream geek(num);
        geek >> value;
    }
    int getVal(){
        return value;
    }
};

class Call: public Node{
public:
    explicit Call(string value){
        type = value;
    }
};
#define YYSTYPE Node*

/*
 **************************************************************
 ********************symbols table*****************************
 **************************************************************
*/

class VarDecleration{
    string name;
    string type;
    int offset;

public:
    VarDecleration(string new_name, string new_type, int new_offset) : name(new_name), type(new_type), offset(new_offset){}

    string getName(){
        return name;
    }
    string getType(){
        return type;
    }
    void setOffset(int new_offset){
        offset = new_offset;
    }
    int getOffset(){
        return offset;
    }
};

class FuncDecleration{
    string name;
    string retType;
    vector<string> argsType;
public:
    FuncDecleration(string func_name, string ret_type, vector<string> args_type) : name(func_name), retType(ret_type),
                                                                                   argsType(args_type){}
    string getName(){
        return name;
    }
    string getType(){
        return retType;
    }
    vector<string> getArgs(){
        return argsType;
    }
};


class symbolsTable{
    vector<int>* offsets;
    vector<vector<VarDecleration>>* names;

public:
    symbolsTable() : offsets(new vector<int>), names(new vector<vector<VarDecleration>>()){}

    /*
      This function inserts a new variable to the symbols table, it could be
      a new variable declared in the scope or a function argument.
    */
    void Insert(const string& name, const string& type, bool funcArg = false){
        if(offsets->empty()){
            if(funcArg){
                offsets->push_back(-1);
            } else{
                offsets->push_back(0);
            }
        }
        VarDecleration decleration(name,type, offsets->back());
        if(offsets->back() < 0){
            offsets->back() -= 1;
        } else{
            offsets->back() += 1;
        }
        names->back().push_back(decleration);
    }

    void closeScope(){
        for (auto & i : names->back()) {
            output::printID(i.getName(), i.getOffset(), i.getType());
        }
        offsets->pop_back();
        names->pop_back();
    }

    void openScope(){
        vector<VarDecleration> new_vec;
        names->push_back(new_vec);
        if(offsets->empty()){
            return;
        }
        if(offsets->back() < 0){
            offsets->push_back(0);
        } else{
            offsets->push_back(offsets->back()+1);
        }
    }

    /*
       This function checks if a variable with var_name was declared in outer scopes and returns its type.
       Returns an empty string if wasn't found.
    */
    string checkVariableDeclared(const string& var_name){
        string var_type;
        for(int i= names->size()-1 ; i>=0 ; i--){
            for(int j=0; j<= names[i].size() ; j++){
                if((*names)[i][j].getName() == var_name){
                    var_type = (*names)[i][j].getType();
                }
            }
        }
        return var_type;
    }

};

class FuncsTable{
    vector<FuncDecleration>* functions;
public:
    FuncsTable() : functions(new vector<FuncDecleration>()) {
        vector<string> print_vec{"STRING"};
        vector<string> printi_vec{"INT"};
        Insert("print", "VOID", print_vec);
        Insert("printi", "VOID", printi_vec);
    }
    void Insert(string name, string ret_type, vector<string> args_type)  {
        FuncDecleration function = FuncDecleration(name, ret_type, args_type);
        functions->push_back(function);
    }


    /*
     * This function checks if the arguments supplied match with the called function arguments,
     * and if the function exists.
     * returned value: return type of the called function
     */
    string checkArgsValid(const string& func_name, vector<string> args){
        bool exists = false;
        string ret_type;
        for (auto & function : *functions) {
            if(function.getName() == func_name){
                exists = true;
                ret_type = function.getType();
                for (int j = 0; j < function.getArgs().size(); ++j) {
                    if(args[j] != (function.getArgs())[j]){
                        vector<string> args_v = function.getArgs();
                        output::errorPrototypeMismatch(yylineno, func_name, args_v);
                        exit(0);
                    }
                }
            }
        }
        if(!exists){
            output::errorUndefFunc(yylineno, func_name);
            exit(0);
        }
        return ret_type;
    }
    string lastFuncType(){
        return functions->back().getType();
    }

    void printTable(){
        for (auto & function : *functions) {
            vector<string> args_v = function.getArgs();
            string func_type = output::makeFunctionType(function.getType(), args_v);
            output::printID(function.getName(),0,func_type);
        }
    }

};





#endif