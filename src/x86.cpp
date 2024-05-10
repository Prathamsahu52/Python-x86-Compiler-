#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <map>
#include "node.cpp"

using namespace std;


vector<vector<threeac*>> all_subroutines;

// int num_params = 0;
stack<int>num_params_stack;

struct instruction{
    string opcode = "";
    string operand1 = "";
    string operand2 = "";
    string operand3 = "";

    string code = "";
    string comment = "";
    bool is_label = false;
};

struct subroutine_entry{
    string name = "";
    int offset = 0; //from base pointer in subroutine
};

struct subroutine_table{
    string subroutine_name;
    bool is_main = false;
    map<string, subroutine_entry*> lookup_table;
    int total_space;
    int num_params = 0;
};
vector<subroutine_table*> all_subroutine_tables;
vector<instruction*> x86_code_final;

// get subroutines from the 3ac
void get_tac_subroutines(node_ast* root){
    vector<threeac*> subroutine;

    bool func_start = false;

    for(int i=0;i<root->code.size();i++){
        if(root->code[i]->op == "beginfunc"){
            func_start = true;
            subroutine.push_back(root->code[i-1]);
            subroutine.push_back(root->code[i]);
        }
        else if(root->code[i]->op == "endfunc"){
            func_start = false;
            subroutine.push_back(root->code[i]);
            // subroutine.push_back(root->code[i+1]);
            all_subroutines.push_back(subroutine);
            subroutine.clear();
        }
        else if(func_start){
            subroutine.push_back(root->code[i]);
        }
        
    }
}

void print_tac_subroutines(){
    for(int i=0;i<all_subroutines.size();i++){
        cout << "Subroutine " << i << endl;
        for(int j=0;j<all_subroutines[i].size();j++){
            cout << all_subroutines[i][j]->op << " " << all_subroutines[i][j]->arg1 << " " << all_subroutines[i][j]->arg2 << " " << all_subroutines[i][j]->result << endl;
        }
    }
}

struct subroutine_table* create_subroutine_table(){
    // subroutine_table* table = (subroutine_table*)malloc(sizeof(subroutine_table));
    subroutine_table* table = new subroutine_table();
    table->lookup_table = map<string, subroutine_entry*>();
    table->subroutine_name = "";
    table->is_main = false;
    table->total_space = 0;
    table->num_params = 0;
    return table;

}

subroutine_entry* create_subroutine_entry(){
    // subroutine_entry* entry = (subroutine_entry*)malloc(sizeof(subroutine_entry));
    subroutine_entry* entry = new subroutine_entry();
    entry->name = "";
    entry->offset = 0;
    return entry;
}
bool isVariable(string arg){
    if(arg==""){
        // cerr<<"Empty argument"<<endl;
        return false;
    }

    return !(arg[0]>='0' && arg[0]<='9');
}
//we need to construct the subroutine table for each subroutine
void construct_subroutine_table(subroutine_table* table, vector<threeac*> subroutine){
    // cerr<<"Constructing table for subroutine"<<endl;
    int popped_params = 2;//initially for old base pointer and return address
    table->total_space = -40;
    table->subroutine_name = subroutine[0]->label->name;
    if(table->subroutine_name == "global.main"){
        table->is_main = true;
    }
    for(int i=1;i<subroutine.size();i++){
        // cerr<<"Instruction "<<i<<endl;

        if(subroutine[i]->op == "pop_param"){
            // cerr<<"Popped param"<<endl;
           
            if(table->lookup_table.find(subroutine[i]->result)==table->lookup_table.end()){
                subroutine_entry* entry = create_subroutine_entry();
                entry->name = subroutine[i]->result;
                entry->offset = 8*popped_params;
                table->lookup_table[subroutine[i]->result] = entry;
            }
            popped_params++;
        }else{
            // cerr<<subroutine[i]->op<<" "<<subroutine[i]->arg1<<" "<<subroutine[i]->arg2<<" "<<subroutine[i]->result<<endl;
            if(subroutine[i]->arg1!="" && table->lookup_table.find(subroutine[i]->arg1)==table->lookup_table.end() && isVariable(subroutine[i]->arg1)){
                subroutine_entry* entry = create_subroutine_entry();
                entry->name = subroutine[i]->arg1;
                entry->offset = table->total_space;
                table->lookup_table[subroutine[i]->arg1] = entry;
                table->total_space -= 8;
            }
            if(subroutine[i]->arg2!="" && table->lookup_table.find(subroutine[i]->arg2)==table->lookup_table.end() && isVariable(subroutine[i]->arg2)){
                subroutine_entry* entry = create_subroutine_entry();
                entry->name = subroutine[i]->arg2;
                entry->offset = table->total_space;
                table->lookup_table[subroutine[i]->arg2] = entry;
                table->total_space -= 8;
            }
            if(subroutine[i]->result!="" && table->lookup_table.find(subroutine[i]->result)==table->lookup_table.end() && isVariable(subroutine[i]->result)){
                // cerr<<"Adding result"<<endl;
                subroutine_entry* entry = create_subroutine_entry();
                entry->name = subroutine[i]->result;
                entry->offset = table->total_space;
                table->lookup_table[subroutine[i]->result] = entry;
                // cerr<<"Offset: "<<entry->offset<<endl;
                table->total_space -= 8;
                // cerr<<"Total space: "<<table->total_space<<endl;
            }
        }
        
    }

}

instruction* create_instruction(string opcode, string operand1, string operand2){
    instruction* inst = new instruction();
    inst->opcode = opcode;
    inst->operand1 = operand1;
    inst->operand2 = operand2;
    inst->code = inst->opcode + " " + inst->operand1 + ", " + inst->operand2;
    return inst;
}
//overloaded
instruction* create_instruction(string opcode, string operand1){
    instruction* inst = new instruction();
    inst->opcode = opcode;
    inst->operand1 = operand1;
    inst->code = inst->opcode + " " + inst->operand1;
    return inst;
}

instruction* create_instruction(string opcode){
    instruction* inst = new instruction();
    inst->opcode = opcode;
    inst->code = inst->opcode;
    return inst;
}

subroutine_table* get_table(string name){
    for(int i=0;i<all_subroutine_tables.size();i++){
            if(all_subroutine_tables[i]->subroutine_name == name ){
                return all_subroutine_tables[i];
            }
    }
    // cerr<<"Function not found"<<endl;
    exit(1);
}
vector<instruction*> make_x86_code(threeac* tac_line, subroutine_table* table, int is_main){
    vector<instruction*> x86_code;
    // instruction* inst = (instruction*)malloc(sizeof(instruction));
    instruction* inst = new instruction();
    // cerr<<"Making x86 code for "<<tac_line->triplet_code<<endl;
  
    if(tac_line->op == "beginfunc"){
        //put the function label before this
        x86_code.push_back(create_instruction("pushq", "%rbp"));
        // inst = (instruction*)malloc(sizeof(instruction));
        x86_code.push_back(create_instruction("movq", "%rsp", "%rbp"));
        //push rbx,rdi,rsi, r12,r13,r14,r15
        x86_code.push_back(create_instruction("pushq", "%rbx"));
        x86_code.push_back(create_instruction("pushq", "%r12"));
        x86_code.push_back(create_instruction("pushq", "%r13"));
        x86_code.push_back(create_instruction("pushq", "%r14"));
        x86_code.push_back(create_instruction("pushq", "%r15"));

        //make space for the temporary variables?? 
        if(table->total_space < 0){
            x86_code.push_back(create_instruction("subq", "$" + to_string(-1*table->total_space), "%rsp"));
        } 

    }
    else if(tac_line->op == "endfunc"){

        if(table->is_main){

            //syscall 60 is just exit
            x86_code.push_back(create_instruction("movq", "$60", "%rax")); 
            x86_code.push_back(create_instruction("xorq", "%rdi", "%rdi"));
            x86_code.push_back(create_instruction("syscall"));

        }else{
        //perform cleanup
            x86_code.push_back(create_instruction("addq", "$" + to_string(-1*table->total_space), "%rsp"));
            //pop rbx,rdi,rsi, r12,r13,r14,r15
            x86_code.push_back(create_instruction("popq", "%r15"));
            x86_code.push_back(create_instruction("popq", "%r14"));
            x86_code.push_back(create_instruction("popq", "%r13"));
            x86_code.push_back(create_instruction("popq", "%r12"));
            x86_code.push_back(create_instruction("popq", "%rbx"));
            x86_code.push_back(create_instruction("popq", "%rbp"));
        
            // inst = (instruction*)malloc(sizeof(instruction));
            inst = new instruction();
            inst->opcode = "ret";
            inst->code = inst->opcode;
            x86_code.push_back(inst);
        }


    }
    //TODO: handling pops on functions
    // else if(tac_line->op == "pop_param"){
    //     if(!isVariable(tac_line->result)){
    //         cerr<<"Error: pop_param result is not a variable"<<endl;
    //         exit(1);
    //     }else{
    //         x86_code.push_back(create_instruction("popq", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    //     }
        
    // }
    //TODO: Handling binary. c(z) = a(z) op b(z)
    else if(tac_line->op == "+"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rdx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rdx"));
        }
        // inst = (instruction*)malloc(sizeof(instruction));
        inst = new instruction();
        //add and load in single instruction and store result in rdx
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("add", "$" + tac_line->arg2, "%rdx"));

        }else{
            x86_code.push_back(create_instruction("add", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rdx"));
        }

        x86_code.push_back(create_instruction("movq", "%rdx", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));

    }else if(tac_line->op == "-"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rdx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rdx"));
        }
        inst = new instruction();
        //add and load in single instruction and store result in rdx
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("sub", "$" + tac_line->arg2, "%rdx"));

        }else{
            x86_code.push_back(create_instruction("sub", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rdx"));
        }

        x86_code.push_back(create_instruction("movq", "%rdx", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));

    }else if(tac_line->op == "*"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rdx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rdx"));
        }
        inst = new instruction();
        //add and load in single instruction and store result in rdx
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("imul", "$" + tac_line->arg2, "%rdx"));

        }else{
            x86_code.push_back(create_instruction("imul", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rdx"));
        }

        x86_code.push_back(create_instruction("movq", "%rdx", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }
    else if(tac_line->op == "/"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rax"));
        }
        //add and load in single instruction and store result in rdx
        inst = new instruction();
        inst->opcode = "cqto";
        inst->code = inst->opcode;
        x86_code.push_back(inst);
        
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        // cerr<<"DIVISION"<<endl;
        inst = new instruction();
        inst->opcode = "idiv";
        inst->operand1 = "%rcx";
        inst->code = inst->opcode + " " + inst->operand1;
        x86_code.push_back(inst);
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));

    }
    else if(tac_line->op == "%"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rax"));
        }
        inst = new instruction();
        inst->opcode = "cqto";
        inst->code = inst->opcode;
        x86_code.push_back(inst);
        
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        
        inst = new instruction();
        inst->opcode = "idiv";
        inst->operand1 = "%rcx";
        inst->code = inst->opcode + " " + inst->operand1;
        x86_code.push_back(inst);
        x86_code.push_back(create_instruction("movq", "%rdx", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "<<"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rax"));
        }
        
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        inst = new instruction();
        inst->opcode = "shl";
        inst->operand1 = "%cl";
        inst->operand2 = "%rax";
        inst->code = inst->opcode + " " + inst->operand1 + ", " + inst->operand2;
        x86_code.push_back(inst);
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == ">>"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rax"));
        }
        
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        inst = new instruction();
        inst->opcode = "sar";
        inst->operand1 = "%cl";
        inst->operand2 = "%rax";
        inst->code = inst->opcode + " " + inst->operand1 + ", " + inst->operand2;
        x86_code.push_back(inst);
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "&"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rax"));
        }
        
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("and", "%rcx", "%rax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op =="|"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rax"));
        }
        
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("or", "%rcx", "%rax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "^"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rax"));
        }
        
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("xor", "%rcx", "%rax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "|"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rax"));
        }
        
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("or", "%rcx", "%rax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "!"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rax"));
        }
        x86_code.push_back(create_instruction("not", "%rax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "**"){

        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rax"));
        }
        //this is power op
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }

        //x86_code for power operation with first operand in rax and second in rcx

        x86_code.push_back(create_instruction("movq", "%rax", "%rdx"));
        x86_code.push_back(create_instruction("movq", "%rcx", "%rcx"));
        x86_code.push_back(create_instruction("movq", "$1", "%rax"));
        x86_code.push_back(create_instruction("cmpq", "$0", "%rcx"));
        x86_code.push_back(create_instruction("je", "power_end"));
        x86_code.push_back(create_instruction("power_loop:"));
        x86_code.push_back(create_instruction("imul", "%rdx", "%rax"));
        x86_code.push_back(create_instruction("dec", "%rcx"));
        x86_code.push_back(create_instruction("cmpq", "$0", "%rcx"));
        x86_code.push_back(create_instruction("jne", "power_loop"));
        x86_code.push_back(create_instruction("power_end:"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));

        


        

        
    }
    //TODO: handling load and stores


    //TODO: HANDLING UNARy
    else if(tac_line->op == "=" && tac_line->equal_type =="STR_ASSIGN"){
        //use lea to load the address in arg1 and then assign it to the result
        x86_code.push_back(create_instruction("lea", tac_line->arg1 + "(%rip)", "%rax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }

    else if(tac_line->op == "=" && tac_line->equal_type =="UNARY_PLUS"){

        //arg1 is +x
        string arg1 = tac_line->arg1.substr(1);
        if(!isVariable(arg1)){
            x86_code.push_back(create_instruction("movq", "$" + arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[arg1]->offset) + "(%rbp)", "%rax"));
        }
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));

    }else if(tac_line->op == "=" && tac_line->equal_type == "UNARY_MINUS"){
        string arg1 = tac_line->arg1.substr(1);
        
        if(!isVariable(arg1)){
            x86_code.push_back(create_instruction("movq", "$" + arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[arg1]->offset) + "(%rbp)", "%rax"));
        }
        x86_code.push_back(create_instruction("neg", "%rax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "=" && tac_line->equal_type == "NOT"){
        
        //FIX THIS???? correctness issue
        string arg1 = tac_line->arg1.substr(1);
        if(!isVariable(arg1)){
            x86_code.push_back(create_instruction("movq", "$" + arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[arg1]->offset) + "(%rbp)", "%rax"));
        }
        x86_code.push_back(create_instruction("not", "%rax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    
    }//TODO: handle stores t = *x and loads *t=x
    else if(tac_line->op == "=" && tac_line->equal_type =="LOAD"){
        string arg1 = tac_line->arg1.substr(1);
        if(!isVariable(arg1)){
            x86_code.push_back(create_instruction("movq", "$" + arg1, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[arg1]->offset) + "(%rbp)", "%rax"));
        }
        x86_code.push_back(create_instruction("movq", "(%rax)", "%rdx"));
        x86_code.push_back(create_instruction("movq", "%rdx", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));

    }else if(tac_line->op == "=" && tac_line->equal_type == "STORE"){
        string result = tac_line->result.substr(1);
        if(!isVariable(result)){
            x86_code.push_back(create_instruction("movq", "$" + result, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[result]->offset) + "(%rbp)", "%rax"));
        }

        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rdx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rdx"));
        }
        x86_code.push_back(create_instruction("movq", "%rdx", "(%rax)"));
        
    }
    else if(tac_line->op == "=" && tac_line->equal_type=="ASSIGN"){//assignment
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
        }else{
            //first push into rdx and then move to result
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rdx"));
            x86_code.push_back(create_instruction("movq", "%rdx", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
        }
    
    }else if(tac_line->op == "call"){

        //if no params

        if(num_params_stack.top() == 0){
            //save caller saved registers rax, rdi, rsi, rdx, rcx, r8, r9,r10, r11
            x86_code.push_back(create_instruction("pushq", "%rax"));
            x86_code.push_back(create_instruction("pushq", "%rdi"));
            x86_code.push_back(create_instruction("pushq", "%rsi"));
            x86_code.push_back(create_instruction("pushq", "%rdx"));
            x86_code.push_back(create_instruction("pushq", "%rcx"));
            x86_code.push_back(create_instruction("pushq", "%r8"));
            x86_code.push_back(create_instruction("pushq", "%r9"));
            x86_code.push_back(create_instruction("pushq", "%r10"));
            x86_code.push_back(create_instruction("pushq", "%r11"));
           
        }
        int local_params = num_params_stack.top();
        num_params_stack.top() = 0;
        inst->opcode = "call";
        inst->operand1 = tac_line->arg1;
        inst->code = inst->opcode + " " + inst->operand1;
        x86_code.push_back(inst);

        if(tac_line->arg1 == "global.print"){
            // dealing specially with print
            inst = new instruction();
            inst->opcode = "addq";
            inst->operand1 = "$8";
            inst->operand2 = "%rsp";
            inst->code = inst->opcode + " " + inst->operand1 + ", " + inst->operand2;
            x86_code.push_back(inst);
            
        }
        else if(tac_line->arg1 == "memalloc"){
            x86_code.push_back(create_instruction("add", "$8", "%rsp"));
        }else{
            //the pop must be correct
            // local_params = get_table(tac_line->arg1)->num_params;
            
            x86_code.push_back(create_instruction("addq","$"+to_string(8*local_params),"%rsp"));
        }
        //TODO: pop the parameters
        

        
    }
    //TODO: Handle return from caller side
    else if(tac_line->op =="ret"){

        if(tac_line->result!=""){
            x86_code.push_back(create_instruction("movq", "%rax",to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
        }
        
        //restore the regs pushed  during the function call
        x86_code.push_back(create_instruction("popq", "%r11"));
        x86_code.push_back(create_instruction("popq", "%r10"));
        x86_code.push_back(create_instruction("popq", "%r9"));
        x86_code.push_back(create_instruction("popq", "%r8"));
        x86_code.push_back(create_instruction("popq", "%rcx"));
        x86_code.push_back(create_instruction("popq", "%rdx"));
        x86_code.push_back(create_instruction("popq", "%rsi"));
        x86_code.push_back(create_instruction("popq", "%rdi"));
        x86_code.push_back(create_instruction("popq", "%rax"));
        if(print_string){
                //put 1 in rdi
                x86_code.push_back(create_instruction("movq", "$1", "%rdi"));
                // cerr<<"PRINTSTRING"<<endl;
                print_string--;
        }

        // if(num_params > 0){
        //     x86_code.push_back(create_instruction("addq", "$" + to_string(8*num_params), "%rsp"));
        //     num_params = 0;
        // }



    }
    //TODO: handle return from the callee
    else if(tac_line->op == "RETURN"){
        if(!isVariable(tac_line->result)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->result, "%rax"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)", "%rax"));
        }
        //perform cleanup

    }else if(tac_line->op == "return"){
        x86_code.push_back(create_instruction("addq", "$" + to_string(-1*table->total_space), "%rsp"));
            //pop rbx,rdi,rsi, r12,r13,r14,r15
        x86_code.push_back(create_instruction("popq", "%r15"));
        x86_code.push_back(create_instruction("popq", "%r14"));
        x86_code.push_back(create_instruction("popq", "%r13"));
        x86_code.push_back(create_instruction("popq", "%r12"));
        x86_code.push_back(create_instruction("popq", "%rbx"));
        x86_code.push_back(create_instruction("popq", "%rbp"));
        instruction* inst = new instruction();
        inst->opcode = "ret";
        inst->code = inst->opcode;
        x86_code.push_back(inst);
        
    }
    //TODO: for parameters pushed
    else if(tac_line->op == "param"){
        //we need to perform the duties if first time we are seeing the parameter
        if(num_params_stack.top() == 0){
            //save caller saved registers rax, rdi, rsi, rdx, rcx, r8, r9,r10, r11
            x86_code.push_back(create_instruction("pushq", "%rax"));
            x86_code.push_back(create_instruction("pushq", "%rdi"));
            x86_code.push_back(create_instruction("pushq", "%rsi"));
            x86_code.push_back(create_instruction("pushq", "%rdx"));
            x86_code.push_back(create_instruction("pushq", "%rcx"));
            x86_code.push_back(create_instruction("pushq", "%r8"));
            x86_code.push_back(create_instruction("pushq", "%r9"));
            x86_code.push_back(create_instruction("pushq", "%r10"));
            x86_code.push_back(create_instruction("pushq", "%r11"));
        }
        if(!isVariable(tac_line->result)){
            inst->opcode = "pushq";
            inst->operand1 = "$" + tac_line->result;
            inst->code = inst->opcode + " " + inst->operand1;
            x86_code.push_back(inst);
        }else{
            inst->opcode = "pushq";
            inst->operand1 = to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)";
            inst->code = inst->opcode + " " + inst->operand1;
            x86_code.push_back(inst);
        }
        num_params_stack.top()++;
    }
    //generate code for label
    else if(tac_line->label !=NULL){
        if(tac_line->label->name[0] == 'L'){
            inst->code = tac_line->label->name + ":";
            inst->is_label = true;
            x86_code.push_back(inst);
        }
    }else if(tac_line->op == "goto"){
        inst->code = "jmp " + tac_line->result;
        x86_code.push_back(inst);
    }
    //TODO: generate for conditional jumps
    else if(tac_line->op == "if"){
        //arg1 is the condition like x LT y, x GT y etc
        //arg2 is the goto
        //result is the label
        string condition = tac_line->condition;

        if(condition == "NOT_COMP"){
            //if the condition is not a comparison, check with 0 and jump
            if(!isVariable(tac_line->arg1)){
                x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rbx"));
            }else{
                x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rbx"));
            }

            x86_code.push_back(create_instruction("cmp", "$0", "%rbx"));
            x86_code.push_back(create_instruction("jne", tac_line->result));
            return x86_code;
        }
        string goto_label = tac_line->result;
        string comp1 = tac_line->comp1;
        string comp2 = tac_line->comp2;

        if(!isVariable(comp1)){
            x86_code.push_back(create_instruction("movq", "$" + comp1, "%rbx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[comp1]->offset) + "(%rbp)", "%rbx"));
        }
        if(!isVariable(comp2)){
            x86_code.push_back(create_instruction("movq", "$" + comp2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[comp2]->offset) + "(%rbp)", "%rcx"));
        }
        if(!tac_line->is_comp_str){
            if(condition == "LT"){
                x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
                x86_code.push_back(create_instruction("jl", goto_label));
            }else if(condition == "GT"){
                x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
                x86_code.push_back(create_instruction("jg", goto_label));
            }else if(condition == "LE"){
                x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
                x86_code.push_back(create_instruction("jle", goto_label));
            }else if(condition == "GE"){
                x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
                x86_code.push_back(create_instruction("jge", goto_label));
            }else if(condition == "EQ"){
                x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
                x86_code.push_back(create_instruction("je", goto_label));
            }else if(condition == "NE"){
                x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
                x86_code.push_back(create_instruction("jne", goto_label));
            }
        }else{
            //this is a string comparison, so we will use strcmp. it returns 0 if equal, negative if s1<s2 and positive if s1>s2
            if(condition == "LT"){
                x86_code.push_back(create_instruction("movq", "%rbx", "%rdi"));
                x86_code.push_back(create_instruction("movq", "%rcx", "%rsi"));
                x86_code.push_back(create_instruction("call", "strcmp"));
                //movsx eax to rax
                x86_code.push_back(create_instruction("movsx", "%eax", "%rax"));
                x86_code.push_back(create_instruction("cmp", "$0", "%rax"));
                x86_code.push_back(create_instruction("jl", goto_label));
            }else if(condition == "GT"){
                x86_code.push_back(create_instruction("movq", "%rbx", "%rdi"));
                x86_code.push_back(create_instruction("movq", "%rcx", "%rsi"));
                x86_code.push_back(create_instruction("call", "strcmp"));
                x86_code.push_back(create_instruction("movsx", "%eax", "%rax"));
                x86_code.push_back(create_instruction("cmp", "$0", "%rax"));
                x86_code.push_back(create_instruction("jg", goto_label));
            }else if(condition == "LE"){
                x86_code.push_back(create_instruction("movq", "%rbx", "%rdi"));
                x86_code.push_back(create_instruction("movq", "%rcx", "%rsi"));
                x86_code.push_back(create_instruction("call", "strcmp"));
                x86_code.push_back(create_instruction("movsx", "%eax", "%rax"));
                x86_code.push_back(create_instruction("cmp", "$0", "%rax"));
                x86_code.push_back(create_instruction("jle", goto_label));
            }else if(condition == "GE"){
                x86_code.push_back(create_instruction("movq", "%rbx", "%rdi"));
                x86_code.push_back(create_instruction("movq", "%rcx", "%rsi"));
                x86_code.push_back(create_instruction("call", "strcmp"));
                x86_code.push_back(create_instruction("movsx", "%eax", "%rax"));
                x86_code.push_back(create_instruction("cmp", "$0", "%rax"));
                x86_code.push_back(create_instruction("jge", goto_label));
            }else if(condition == "EQ"){
                x86_code.push_back(create_instruction("movq", "%rbx", "%rdi"));
                x86_code.push_back(create_instruction("movq", "%rcx", "%rsi"));
                x86_code.push_back(create_instruction("call", "strcmp"));
                x86_code.push_back(create_instruction("movsx", "%eax", "%rax"));
                x86_code.push_back(create_instruction("cmp", "$0", "%rax"));
                x86_code.push_back(create_instruction("je", goto_label));
            }else if(condition == "NE"){
                x86_code.push_back(create_instruction("movq", "%rbx", "%rdi"));
                x86_code.push_back(create_instruction("movq", "%rcx", "%rsi"));
                x86_code.push_back(create_instruction("call", "strcmp"));
                x86_code.push_back(create_instruction("movsx", "%eax", "%rax"));
                x86_code.push_back(create_instruction("cmp", "$0", "%rax"));
                x86_code.push_back(create_instruction("jne", goto_label));
            }

        }


    }else if(tac_line->op == "LT"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rbx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rbx"));
        }
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
        x86_code.push_back(create_instruction("setl", "%al"));
        x86_code.push_back(create_instruction("movzbl", "%al", "%eax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "GT"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rbx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rbx"));
        }
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
        x86_code.push_back(create_instruction("setg", "%al"));
        x86_code.push_back(create_instruction("movzbl", "%al", "%eax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "LE"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rbx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rbx"));
        }
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
        x86_code.push_back(create_instruction("setle", "%al"));
        x86_code.push_back(create_instruction("movzbl", "%al", "%eax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "GE"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rbx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rbx"));
        }
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
        x86_code.push_back(create_instruction("setge", "%al"));
        x86_code.push_back(create_instruction("movzbl", "%al", "%eax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "EQ"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rbx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rbx"));
        }
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
        x86_code.push_back(create_instruction("sete", "%al"));
        x86_code.push_back(create_instruction("movzbl", "%al", "%eax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "NE"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rbx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rbx"));
        }
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("cmp", "%rcx", "%rbx"));
        x86_code.push_back(create_instruction("setne", "%al"));
        x86_code.push_back(create_instruction("movzbl", "%al", "%eax"));
        x86_code.push_back(create_instruction("movq", "%rax", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    
    //else if(tac_line->op == "NOT"){
    //     if(!isVariable(tac_line->arg1)){
    //         x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rbx"));
    //     }else{
    //         x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rbx"));
    //     }
    //     x86_code.push_back(create_instruction("not", "%rbx"));
    //     x86_code.push_back(create_instruction("movq", "%rbx", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "AND"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rbx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rbx"));
        }
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("and", "%rcx", "%rbx"));
        x86_code.push_back(create_instruction("movq", "%rbx", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "OR"){
        if(!isVariable(tac_line->arg1)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg1, "%rbx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg1]->offset) + "(%rbp)", "%rbx"));
        }
        if(!isVariable(tac_line->arg2)){
            x86_code.push_back(create_instruction("movq", "$" + tac_line->arg2, "%rcx"));
        }else{
            x86_code.push_back(create_instruction("movq", to_string(table->lookup_table[tac_line->arg2]->offset) + "(%rbp)", "%rcx"));
        }
        x86_code.push_back(create_instruction("or", "%rcx", "%rbx"));
        x86_code.push_back(create_instruction("movq", "%rbx", to_string(table->lookup_table[tac_line->result]->offset) + "(%rbp)"));
    }else if(tac_line->op == "print_string"){
        //push 1 to rdi
        x86_code.push_back(create_instruction("movq", "$0", "%rdi"));
        print_string++;
        
    }

    return x86_code;
}


vector<instruction*> make_x86_code_from_subroutines(vector<threeac*> subroutine, subroutine_table* table){
    vector<instruction*> x86_code;
    num_params_stack.push(0);
    instruction* inst = new instruction();
    inst->code = (table->is_main?"main":table->subroutine_name)+ ":";
    inst->is_label = true;
    x86_code.push_back(inst);
    for(int i=1;i<subroutine.size();i++){
        vector<instruction*> x86_inst = make_x86_code(subroutine[i], table, table->is_main);
        for(int j=0;j<x86_inst.size();j++){
            x86_code.push_back(x86_inst[j]);
        }
    }
    num_params_stack.pop();
    return x86_code;
}
vector<instruction*> generate_global_data_section(){
    vector<instruction*> x86_code_global;
    // instruction inst = (instruction)malloc(sizeof(instruction));
    instruction* inst = new instruction();
    inst->code = ".data";
    inst->comment = "#Global data section";
    x86_code_global.push_back(inst);
    // inst = (instruction*)malloc(sizeof(instruction));
    inst = new instruction();
    inst->code = "integer_format: .asciz \"%d\\n\"";
    inst->comment = "#Format string for printing integers";
    x86_code_global.push_back(inst);
    inst = new instruction();
    inst->code = "string_format: .asciz \"%s\\n\"";
    inst->comment = "#Format string for printing strings";
    x86_code_global.push_back(inst);

    //now loop through the label_to_string map and add the strings to the data section
    for(auto it = label_to_string.begin(); it!=label_to_string.end();it++){
        // inst = (instruction*)malloc(sizeof(instruction));
        inst = new instruction();
        //first push back the label
        inst->code = it->first + ":";
        inst->is_label = true;
        x86_code_global.push_back(inst);

        inst = new instruction();
        inst->code = "\t.string " + it->second;
        x86_code_global.push_back(inst);
        

    }
    // inst = (instruction*)malloc(sizeof(instruction));
    inst = new instruction();
    inst->code = ".global main";
    inst->is_label = true;
    inst->comment = "#Global main function";
    x86_code_global.push_back(inst);
    inst = new instruction();
    inst->code = ".text";
    inst->is_label = true;
    inst->comment = "#Text section";
    x86_code_global.push_back(inst);


    return x86_code_global;
}

void print_code(vector<instruction*> x86_code){
    ofstream x86_file;
    x86_file.open("x86_code.s");

    /// For the global data section
    vector<instruction*> x86_code_global = generate_global_data_section();
    for(int i=0;i<x86_code_global.size();i++){
        if(x86_code[i]->is_label){
            x86_file << x86_code_global[i]->code << endl;
        }
        else if(x86_code[i]->code[0] != '.'){
            x86_file << "\t" << x86_code_global[i]->code << endl;
        }
        else{
            x86_file << x86_code_global[i]->code << endl;
        }
    }

    for(int i=0;i<x86_code.size();i++){
        //if it doesnt start with . then give 2 tabs
        if(x86_code[i]->is_label){
            x86_file << x86_code[i]->code << endl;
        }
        else if(x86_code[i]->code[0] != '.'){
            x86_file << "\t" << x86_code[i]->code << endl;
        }
        else{
            x86_file << x86_code[i]->code << endl;
        }
    
    }
    //add the print function from print.s
    ifstream print_file;
    print_file.open("src/print.s");
    string line;
    while(getline(print_file, line)){
        x86_file << line << endl;
    }

    ifstream allocmem_file;
    allocmem_file.open("src/allocmem.s");
    while(getline(allocmem_file, line)){
        x86_file << line << endl;
    }
    x86_file.close();

}

void generate_text_section(node_ast* root){

    get_tac_subroutines(root);
    // print_tac_subroutines();
    
    for(int i=0;i<all_subroutines.size();i++){
        subroutine_table* sub_table = create_subroutine_table();
       
        construct_subroutine_table(sub_table, all_subroutines[i]);
        all_subroutine_tables.push_back(sub_table);
    }

    for(int i=0;i<all_subroutines.size();i++){
        vector<instruction*> x86_code = make_x86_code_from_subroutines(all_subroutines[i], all_subroutine_tables[i]);
        
        for(int j=0;j<x86_code.size();j++){
            // cout << x86_code[j]->code << endl;
            x86_code_final.push_back(x86_code[j]);
        
        }
    }
    print_code(x86_code_final);

    
}

