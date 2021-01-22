#ifndef _TYPES_
#define _TYPES_

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "hw3_output.hpp"
#include "generator.hpp"
using namespace std;
extern int yylineno;

class Node {
public:
    string name;
    string type;
    virtual void makeItPolymorphic(){}
    virtual string getName(){
        return name;
    }
    virtual string getType(){
        return type;
    }
    virtual void setName(string new_name){
        name = new_name;
    }
    virtual void setType(string new_type){
        type = new_type;
    }
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

/* new class*/

class String: public Node{ //NEW
    string value;
    int size;
public:
    explicit String(string str) {
        value = str;
        size=value.size();
    }
    string getValue(){return value;}
    int getSize(){return size;}

};

class Exp: public Node{
   
public:
    string reg;
    vector<pair<int,BranchLabelIndex>> falseList, trueList; //NEW: added trueList and FalseList for when the exp is BOOL
    explicit Exp(string type, string reg){
        this->type = type;
        this->reg = reg;
        //falseList.clear();
       // trueList.clear();
    }
    Exp(Exp* exp){ //TODO: add/remove if needed.
      this->name= exp->name;
      this->type=exp->type;
      this->reg=exp->reg;
      this->falseList=exp->falseList;
      this->trueList=exp->trueList;
    }
     void addToFalseList(pair<int,BranchLabelIndex> branch);//TODO: complete the implementation
     void addToTrueList(pair<int,BranchLabelIndex> branch);

};

class Num : public Node{
    string value;
public:
    explicit Num(string num){
        type = "INT";
        name = num;
        stringstream geek(num); 
        geek >> value;
    }
    string getVal(){
        return value;
    }
};

class Call: public Node{
    
public:
    string reg;
    explicit Call(string value,int reg){
        type = value;
        this->reg=reg;
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
    void Insert(const string name, const string type, bool funcArg = false){

        if(offsets->empty()){
            if(funcArg){
                offsets->push_back(-1);
            } else{
                offsets->push_back(0);
            }
            if(names->empty()) {
                vector<VarDecleration> new_vec;
                names->push_back(new_vec);
            }

        }
        if(!funcArg && offsets->back() < 0){
            offsets->back()=0;
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
        output::endScope();
        for (auto & i : names->back()) {
            output::printID(i.getName(), i.getOffset(), i.getType());
        }
        if(offsets->size()) {
            offsets->pop_back();
            names->pop_back();
        }

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
            offsets->push_back(offsets->back());
        }
    }

    /*
       This function checks if a variable with var_name was declared in outer scopes and returns its type.
       Returns an empty string if wasn't found.
    */
    string checkVariableDeclared(const string& var_name){

        string var_type;
        for(int i= names->size()-1 ; i>=0 ; i--){
            for(auto & j : (*names)[i]){
                if(j.getName() == var_name){
                    var_type = j.getType();
                }
            }
        }
        return var_type;
    }

    int varGetOffset(const string& var_name){

        int var_off = 0;
        for(int i= names->size()-1 ; i>=0 ; i--){
            for(auto & j : (*names)[i]){
                if(j.getName() == var_name){
                    var_off = j.getOffset();
                }
            }
        }
        return var_off;
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
        /*if(name == "main" && (ret_type != "VOID" || args_type.size() !=0)){
            output::endScope();
            output::errorMainMissing();
            exit(0);
        }*/
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
        vector<string> reversed;
        for (int i = args.size()-1; i >=0 ; --i) {
            reversed.push_back(args[i]);
        }
        for (auto & function : *functions) {
            if(function.getName() == func_name){
                if(function.getArgs().size() != args.size()){
                    vector<string> args_vec = function.getArgs();
                    output::errorPrototypeMismatch(yylineno, func_name, args_vec);
                    exit(0);
                }
                exists = true;
                ret_type = function.getType();
                for (int j = 0; j < function.getArgs().size(); ++j) {
                    if(reversed[j] != (function.getArgs())[j] && !((function.getArgs())[j]=="INT" && reversed[j] == "BYTE")){
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

    bool checkMain(){
        for (auto & function : *functions) {
            if(function.getName() == "main" && function.getType()== "VOID" && function.getArgs().empty()){
                return true;
            }
        }
        return false;
    }

    bool funcExists(string name){
        for (int i = 0; i <functions->size() ; ++i) {
            if((*functions)[i].getName() == name){
                return true;
            }
        }
        return false;
    }

};





#endif