#include <iostream>
#include <string>
#include<stdlib.h>
#include <vector>
#include <stack>
#include <fstream>
#include <cstring>
#include <unordered_map>
using namespace std;
extern int yylineno;
char* input_file;
struct symbol_table;

int offset = 0;

unordered_map <string, int> type_size = {
   {"INT", 8},
   {"FLOAT", 8},
   {"CHAR", 8},
   {"BOOL", 8},
   {"STR", 8}, // will store the address of the string
   {"STRING", 8}, // will store the address of the string
   {"NUMBER", 8},
   {"VOID", 0},
   {"NOTDEFINED", 0},
   {"LIST", 8},
   
};

stack<symbol_table*> table_stack;
struct table_entry {
  string name;
  string type; // return type in case of function
  string token;
  int entry_offset;
  struct symbol_table *symbol_table; //in case the entry is a symbol table, as in the case of a function in a class
                              // however function in global scope is added in children itself
  int line_number;
  string scope;
  bool is_array;
  string array_type;
  int array_size;
  
};

struct symbol_table{
    string table_type; // global, class, function, local, if
    string name;
    string scope;
    vector<table_entry*> entries;
    vector<symbol_table*> children;
    vector<symbol_table*> descendant_classes;
    symbol_table *parent;
    string return_type;
    vector<table_entry*> parameters;
    int scope_start_offset;

    int line_number;
    table_entry* my_table_entry;
    // int scope_end;

};
struct label{
    string name;

};
struct threeac {
    string op;
    string arg1;
    string arg2;
    string result;
    string triplet_code = "";
    int relative_jump = 0;
    int inst_line = 0;
    int absolute_jump = 0;
    struct label* label;
    bool is_target = false;
    bool is_conditional = false;
    string condition = "";
    string comp1;
    string comp2;
    string equal_type;

    bool is_comp_str = false;
};

vector<string> string_temps;

struct node_ast{
    string name;
    string addr;
    string lexval;
    vector<threeac*> code;
    vector<node_ast*> children;
    symbol_table* symbol_t;
    table_entry* entry;
    table_entry* array_entry;
    table_entry* func_entry;
    vector<table_entry*> entries;
    string type;
    bool arr_arg = false;
    bool type_set = false;
    string array_type;
    struct label* label_true;
    struct label* label_false;
    struct label* label_next;
    struct label* label_begin;
    struct label* label_iter;

    bool is_dot = false;
    string dot_name;
    string dot_obj;

    bool is_object = false;
    string obj_class_name;

    bool is_func_dot = false;
    string func_dot_name;
    string func_dot_obj;

    bool compare_type_str = false;
};


map<string,int> array_num_elements;   
map<string,string>label_to_string;
map<string,string>string_to_label;


string create_new_string_label(){
    string label = ".str";
    label += to_string(label_to_string.size());
    return label;
}


void add_entry_to_entries(vector<table_entry*> &entries, table_entry *entry){
    //check if the entry already exists
    for(int i = 0; i < entries.size(); i++){
        if(entries[i]->name == entry->name){
            cout << "Error at line no " << entry->line_number << ":Variable " << entry->name << " already exists in the current scope" << endl;
            exit(1);
        }
    }
    entries.push_back(entry);
}
struct table_entry* create_table_entry(string name, string type, symbol_table *symbol_table, int line_number, string token, int offset){
    struct table_entry *entry = new table_entry();
    // cerr<<"creating entry"<<name<<endl;
    entry->name = name;
    entry->type = type;
    entry->symbol_table = symbol_table;
    entry->line_number = line_number;
    entry->token = token;
    entry->entry_offset = offset;
    // cerr<<"offset"<<offset<<endl;
    return entry;
}

struct symbol_table* create_global_table(){
   struct symbol_table *global_table = new symbol_table();
    global_table->table_type = "global";
    global_table->name = "global";
    global_table->scope = "global";
    global_table->parent = NULL;


    //push __name__ to the global table

    struct table_entry *entry = create_table_entry("__name__", "STR", NULL, -1, "GLOBAL", -1);
    add_entry_to_entries(global_table->entries, entry);
    return global_table;
}

struct symbol_table* global_table = create_global_table();

struct symbol_table* create_symbol_table(string table_type){
    struct symbol_table *local_table = new symbol_table();
    local_table->table_type = table_type;
    local_table->scope = "local";
    local_table->entries = vector<table_entry*>();
    local_table->parent = table_stack.top();
    local_table->scope_start_offset = offset;
    return local_table;

}

struct symbol_table *create_symbol_table_function(string name, vector<table_entry*> parameters, string return_type, string type, int line_number){

    //check the scope of the current top of stack if there are two functions with same name
    struct symbol_table *class_table = table_stack.top();
    for(int i = 0; i < class_table->entries.size(); i++){
        if(class_table->entries[i]->symbol_table==NULL){
            continue;
        }
        else if(class_table->entries[i]->name == name){
            cerr << "Error"<<" at line number"<< yylineno<<": Function " << name << " already exists in the class" << endl;
            exit(1);
        }
    }

    struct symbol_table *local_table =new symbol_table();
    local_table->name = name;
    local_table->table_type = type;
    local_table->parameters = parameters;
    local_table->return_type = return_type;
    local_table->scope = "local";
    local_table->entries = vector<table_entry*>();
    local_table->parent = global_table;
    local_table->scope_start_offset = offset;
    local_table->line_number = line_number;

    return local_table;
}

struct symbol_table *create_symbol_table_class(string name, int line_number){
    struct symbol_table *local_table = (struct symbol_table*)malloc(sizeof(symbol_table));
    local_table->name = name;
    local_table->table_type = "class";
    local_table->scope = "local";
    local_table->entries = vector<table_entry*>();
    local_table->parent = global_table;
    local_table->descendant_classes = vector<symbol_table*>();
    local_table->return_type = "NULL";
    local_table->scope_start_offset = offset;
    local_table->line_number = line_number;


    //make an entry and add it to the global symbol table

    struct table_entry *entry = create_table_entry(name, "class", local_table, -1, "CLASSDEF", -1);
    add_entry_to_entries(global_table->entries, entry);

    return local_table;
}


void add_to_global_table(symbol_table *local_table){
    global_table->children.push_back(local_table);
}

void print_global_table(){
    cout<<"PRINTING GLOBAL TABLE"<<endl;
    cout<<"====================="<<endl;
    cout<<"GLOBAL FUNCTIONS and VARIABLES"<<endl;
    
    for(int i = 0; i < global_table->entries.size(); i++){
        cout<<"====================="<<endl;
        if(global_table->entries[i]->symbol_table == NULL){
            cout << "Variable: " << global_table->entries[i]->name << " " << global_table->entries[i]->type <<" offset:"<< global_table->entries[i]->entry_offset<< endl;

            if(global_table->entries[i]->is_array){
                cout<<"Array type:"<<global_table->entries[i]->array_type<<endl;
                cout<<"Array Size: "<<global_table->entries[i]->array_size<<endl;
            }
        }
        else if(global_table->entries[i]->symbol_table->table_type == "function"){
            cout << "FUNCTION: " << global_table->entries[i]->symbol_table->name << endl;
            cout << "Return Type: " << global_table->entries[i]->symbol_table->return_type << endl;
            cout << "Parameters: " << endl;
            for(int j = 0; j < global_table->entries[i]->symbol_table->parameters.size(); j++){
                cout << global_table->entries[i]->symbol_table->parameters[j]->name << " " << global_table->entries[i]->symbol_table->parameters[j]->type << " offset:" << global_table->entries[i]->symbol_table->parameters[j]->entry_offset<< " line number : "<<global_table->entries[i]->symbol_table->parameters[j]->line_number<<endl;
            }

            for(int j = 0; j < global_table->entries[i]->symbol_table->entries.size(); j++){
                cout << "Variable: " << global_table->entries[i]->symbol_table->entries[j]->name << " " << global_table->entries[i]->symbol_table->entries[j]->type << " offset:" << global_table->entries[i]->symbol_table->entries[j]->entry_offset<< "line number : " << global_table->entries[i]->symbol_table->entries[j]->line_number<<endl;
                if(global_table->entries[i]->symbol_table->entries[j]->is_array){
                    cout<<"Array type:"<<global_table->entries[i]->symbol_table->entries[j]->array_type<<endl;
                    cout<<"Array Size: "<<global_table->entries[i]->symbol_table->entries[j]->array_size<<endl;

                }
            }
        }
    }
    cout<<"CLASSES LIST"<<endl;
    for(int i = 0; i < global_table->children.size(); i++){
        cout<<"====================="<<endl;
        if(global_table->children[i]->table_type == "class"){
            cout << "Class: " << global_table->children[i]->name << endl;
            cout<< "Parent: " << global_table->children[i]->parent->name << endl;
            //add all the children in the class which are included in the entries

            for(int j = 0; j < global_table->children[i]->entries.size(); j++){
                cout<<"======="<<endl;
                if(global_table->children[i]->entries[j]->symbol_table == NULL){
                    cout << "Variable: " << global_table->children[i]->entries[j]->name << " " << global_table->children[i]->entries[j]->type << " offset:" << global_table->children[i]->entries[j]->entry_offset<< endl;
                    if(global_table->children[i]->entries[j]->is_array){
                        cout<<"Array Size: "<<global_table->children[i]->entries[j]->entry_offset<<endl;
                    }
                }
                else if(global_table->children[i]->entries[j]->symbol_table->table_type == "function"){
                    cout << "FUNCTION: " << global_table->children[i]->entries[j]->symbol_table->name << endl;
                    cout << "Return Type: " << global_table->children[i]->entries[j]->symbol_table->return_type << endl;
                    cout << "Parameters: " << endl;
                    for(int k = 0; k < global_table->children[i]->entries[j]->symbol_table->parameters.size(); k++){
                        cout << global_table->children[i]->entries[j]->symbol_table->parameters[k]->name << " " << global_table->children[i]->entries[j]->symbol_table->parameters[k]->type << " offset:" << global_table->children[i]->entries[j]->symbol_table->parameters[k]->entry_offset<< endl;
                        if(global_table->children[i]->entries[j]->symbol_table->parameters[k]->is_array){
                            cout<<"Array Size: "<<global_table->children[i]->entries[j]->symbol_table->parameters[k]->entry_offset<<endl;
                        }
                    }

                    for(int k = 0; k < global_table->children[i]->entries[j]->symbol_table->entries.size(); k++){
                        cout << "Variable: " << global_table->children[i]->entries[j]->symbol_table->entries[k]->name << " " << global_table->children[i]->entries[j]->symbol_table->entries[k]->type << " offset:" << global_table->children[i]->entries[j]->symbol_table->entries[k]->entry_offset<< endl;
                        if(global_table->children[i]->entries[j]->symbol_table->entries[k]->is_array){
                            cout<<"Array Size: "<<global_table->children[i]->entries[j]->symbol_table->entries[k]->entry_offset<<endl;
                        }
                    }


                }
            }
        }
        
    }
}

 void writeToCSV(const string& filename, const vector<string>& data) {
    ofstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Unable to open file " << filename << endl;
        return;
    }

    // Write data to the file in CSV format
    for (size_t i = 0; i < data.size(); ++i) {
        file << data[i];
        
        file << "\n";
        
    }

    file.close();
}

void print_csv(){
    vector<string> text;
    text.push_back("Name,Type,Scope,Offset,Local Offset,Line Number,Size,Token,Parameters,Inherits");
    string name, type, scope, offset, local_offset, line_number, size, token, parameters, inherits, line;
    text.push_back("\n");
    for(int i = 0; i < global_table->entries.size(); i++){
        if (global_table->entries[i]->symbol_table == NULL){
            name = global_table->entries[i]->name;
            type = global_table->entries[i]->type;
            scope = "global";
            offset = to_string(global_table->entries[i]->entry_offset);
            local_offset = to_string(global_table->entries[i]->entry_offset - global_table->scope_start_offset);
            line_number = to_string(global_table->entries[i]->line_number);
            size = to_string(type_size[global_table->entries[i]->type]);
            if(global_table->entries[i]->is_array){
                size = to_string(global_table->entries[i]->array_size * type_size[global_table->entries[i]->array_type]);
            }
            token = global_table->entries[i]->token;
            parameters = "NULL";
            inherits = "NULL";
            line = name + "," + type + "," + scope + "," + offset + "," + local_offset + "," + line_number + "," + size + "," + token + "," + parameters + "," + inherits;
            text.push_back(line);
        }
    }
    text.push_back("\n");
    for(int i = 0; i < global_table->entries.size(); i++){
        if (global_table->entries[i]->symbol_table != NULL && global_table->entries[i]->symbol_table->table_type == "function"){
            name = global_table->entries[i]->symbol_table->name + "( Function )";
            type = global_table->entries[i]->symbol_table->return_type;
            scope = "global";
            offset = to_string(global_table->entries[i]->entry_offset);
            local_offset = to_string(global_table->entries[i]->entry_offset - global_table->scope_start_offset);
            line_number = to_string(global_table->entries[i]->line_number);
            size = "NULL"; //TODO: calculate size of function if needed
            token = global_table->entries[i]->token;
            parameters = "";
            for (int j = 0; j < global_table->entries[i]->symbol_table->parameters.size(); j++){
                parameters += global_table->entries[i]->symbol_table->parameters[j]->name + ":" + global_table->entries[i]->symbol_table->parameters[j]->type + " ; ";
            }
            inherits = "NULL";
            line = name + "," + type + "," + scope + "," + offset + "," + local_offset + "," + line_number + "," + size + "," + token + "," + parameters + "," + inherits;
            text.push_back(line);
            // TODO : printing the variables inside this function
            for(int j=0; j<global_table->entries[i]->symbol_table->entries.size(); j++){
                name = global_table->entries[i]->symbol_table->entries[j]->name;
                type = global_table->entries[i]->symbol_table->entries[j]->type;
                scope = global_table->entries[i]->symbol_table->name + "( Function )";
                offset = to_string(global_table->entries[i]->symbol_table->entries[j]->entry_offset);
                local_offset = to_string(global_table->entries[i]->symbol_table->entries[j]->entry_offset - global_table->entries[i]->symbol_table->scope_start_offset);
                line_number = to_string(global_table->entries[i]->symbol_table->entries[j]->line_number);
                size = to_string(type_size[global_table->entries[i]->symbol_table->entries[j]->type]);
                if(global_table->entries[i]->symbol_table->entries[j]->is_array){
                    size = to_string(global_table->entries[i]->symbol_table->entries[j]->array_size * type_size[global_table->entries[i]->symbol_table->entries[j]->array_type]);
                }
                token = global_table->entries[i]->symbol_table->entries[j]->token;
                parameters = "NULL";
                inherits = "NULL";
                line = name + "," + type + "," + scope + "," + offset + "," + local_offset + "," + line_number + "," + size + "," + token + "," + parameters + "," + inherits;
                text.push_back(line);
            }
        }
    }
    //For Classes
    for(int i=0; i<global_table->children.size(); i++){
        if(global_table->children[i]->table_type == "class"){
            text.push_back("\n");
            name = global_table->children[i]->name + "( Class )";
            type = "";
            scope = "global";
            offset = to_string(global_table->children[i]->scope_start_offset);
            local_offset = to_string(global_table->children[i]->scope_start_offset - global_table->scope_start_offset);
            line_number = to_string(global_table->children[i]->line_number); //TODO : add the line number for the class definition
            size = "NULL";
            token = global_table->entries[i]->token;
            parameters = "";
            if (global_table->children[i]->parent->name == "global"){
                inherits = "NULL";
            } else {
                inherits = global_table->children[i]->parent->name;
            }
            line = name + "," + type + "," + scope + "," + offset + "," + local_offset + "," + line_number + "," + size + "," + token + "," + parameters + "," + inherits;
            text.push_back(line);
            //TODO : printing the variables and functions inside this class
            for (int j=0; j<global_table->children[i]->entries.size(); j++){
                if (global_table->children[i]->entries[j]->symbol_table == NULL){
                    name = global_table->children[i]->entries[j]->name;
                    type = global_table->children[i]->entries[j]->type;
                    scope = global_table->children[i]->name + "( Class )";
                    offset = to_string(global_table->children[i]->entries[j]->entry_offset);
                    local_offset = to_string(global_table->children[i]->entries[j]->entry_offset - global_table->children[i]->scope_start_offset);
                    line_number = to_string(global_table->children[i]->entries[j]->line_number);
                    size = to_string(type_size[global_table->children[i]->entries[j]->type]);
                    if(global_table->children[i]->entries[j]->is_array){
                        size = to_string(global_table->children[i]->entries[j]->array_size * type_size[global_table->children[i]->entries[j]->array_type]);
                    }
                    token = global_table->children[i]->entries[j]->token;
                    parameters = "NULL";
                    inherits = "NULL";
                    line = name + "," + type + "," + scope + "," + offset + "," + local_offset + "," + line_number + "," + size + "," + token + "," + parameters + "," + inherits;
                    text.push_back(line);
                }
            }

            for (int j=0; j<global_table->children[i]->entries.size(); j++){
                if (global_table->children[i]->entries[j]->symbol_table != NULL && global_table->children[i]->entries[j]->symbol_table->table_type == "function"){
                    text.push_back("\n");
                    name = global_table->children[i]->entries[j]->symbol_table->name + "( Function )";
                    type = global_table->children[i]->entries[j]->symbol_table->return_type;
                    scope = global_table->children[i]->name + "( Class )";
                    offset = to_string(global_table->children[i]->entries[j]->entry_offset);
                    local_offset = to_string(global_table->children[i]->entries[j]->entry_offset - global_table->children[i]->scope_start_offset);
                    line_number = to_string(global_table->children[i]->entries[j]->line_number);
                    size = "NULL"; //TODO: calculate size of function if needed
                    token = global_table->children[i]->entries[j]->token;
                    parameters = "";
                    for (int k = 0; k < global_table->children[i]->entries[j]->symbol_table->parameters.size(); k++){
                        parameters += global_table->children[i]->entries[j]->symbol_table->parameters[k]->name + ":" + global_table->children[i]->entries[j]->symbol_table->parameters[k]->type + " ; ";
                    }
                    inherits = "NULL";
                    line = name + "," + type + "," + scope + "," + offset + "," + local_offset + "," + line_number + "," + size + "," + token + "," + parameters + "," + inherits;
                    text.push_back(line);
                    // TODO : printing the variables inside this function
                    for(int k=0; k<global_table->children[i]->entries[j]->symbol_table->entries.size(); k++){
                        name = global_table->children[i]->entries[j]->symbol_table->entries[k]->name;
                        type = global_table->children[i]->entries[j]->symbol_table->entries[k]->type;
                        scope = global_table->children[i]->entries[j]->symbol_table->name + "( Function )";
                        offset = to_string(global_table->children[i]->entries[j]->symbol_table->entries[k]->entry_offset);
                        local_offset = to_string(global_table->children[i]->entries[j]->symbol_table->entries[k]->entry_offset - global_table->children[i]->entries[j]->symbol_table->scope_start_offset);
                        line_number = to_string(global_table->children[i]->entries[j]->symbol_table->entries[k]->line_number);
                        size = to_string(type_size[global_table->children[i]->entries[j]->symbol_table->entries[k]->type]);
                        if(global_table->children[i]->entries[j]->symbol_table->entries[k]->is_array){
                            size = to_string(global_table->children[i]->entries[j]->symbol_table->entries[k]->array_size * type_size[global_table->children[i]->entries[j]->symbol_table->entries[k]->array_type]);
                        }
                        token = global_table->children[i]->entries[j]->symbol_table->entries[k]->token;
                        parameters = "NULL";
                        inherits = "NULL";
                        line = name + "," + type + "," + scope + "," + offset + "," + local_offset + "," + line_number + "," + size + "," + token + "," + parameters + "," + inherits;
                        text.push_back(line);
                    }
                }
            }
        }

    }
    string input_file_str = input_file;
    std::size_t found = input_file_str.find_last_of(".");
    string csv_file = input_file_str.substr(0, found) + ".csv";
    writeToCSV(csv_file, text);
}


void add_entry_to_entries_union(vector<table_entry*> &entries, table_entry *entry){
   
    for(int i = 0; i < entries.size(); i++){
        if(entries[i]->name == entry->name){
            entries[i]->type = entry->type;
            entries[i]->line_number = entry->line_number;
            return;
        }
    }
    entries.push_back(entry);

}





void add_entry_to_symbol_table(table_entry *entry){
    //add to top of stack entry
    symbol_table *current_table = table_stack.top();
    // //cerr<<"entering"<<entry->name<<"into"<<current_table->table_type<<endl;
    add_entry_to_entries(current_table->entries, entry);
}



void add_entry_to_symbol_table_union(table_entry *entry){
    symbol_table *current_table = table_stack.top();
    add_entry_to_entries_union(current_table->entries, entry);
    
}

void add_to_parent_table(table_entry *entry){
    symbol_table *current_table = table_stack.top();
    symbol_table *parent_table = current_table->parent;
    add_entry_to_entries(parent_table->entries, entry);
}

void add_entries_to_symbol_table(symbol_table *table, vector<table_entry*> &entries){
    
    for(int i = 0; i < entries.size(); i++){
        add_entry_to_entries(table->entries, entries[i]);
    }
}


int check_if_name_defined(string name){

    //take the top of the stack and check
    symbol_table *current_table = table_stack.top();
    for(int i = 0; i < current_table->entries.size(); i++){
        if(current_table->entries[i]->name == name){
            return 1;
        }
    }
    return 0;
}
// This adds a symbol table(ex. function symbol table) to another symbol table's entries(ex. class)
void add_symbol_table_to_class_entries(symbol_table *entry){
    table_entry *entry1 = create_table_entry(entry->name, entry->return_type, entry, 0,"FUNCDEF", -1);
    // add_entry_to_entries(entries, entry1);
    entry->my_table_entry = entry1;
    add_entry_to_symbol_table(entry1);
}


int bfs_search(symbol_table* current_table, string class_name){
    if(current_table->name == class_name && current_table->table_type == "class"){
        return 1;
    }
    for(int i=0;i<current_table->descendant_classes.size();i++){
        if(bfs_search(current_table->descendant_classes[i], class_name)){
            return 1;
        }
    }   
    return 0;
}
void allocate_parent_and_child(symbol_table* symbol_t){
    
    for(int i=0;i<global_table->children.size();i++){
        if(global_table->children[i]->table_type == "class"){
            if(global_table->children[i]->name == symbol_t->name){
                cerr<<"Error" <<"at line"<<yylineno<<": Class " << symbol_t->name << " already exists and defined again";
                exit(1);
            }
        }
    }
    symbol_t->parent = table_stack.top();
    table_stack.top()->children.push_back(symbol_t);

}

int bfs_search_and_add(symbol_table* current_table, string class_name, symbol_table* descendant_table){
    if(current_table->name == class_name && current_table->table_type == "class"){
        //add the descendant_table to the current_table
        current_table->descendant_classes.push_back(descendant_table);
        descendant_table->parent = current_table;
        return 1;
    }
    for(int i=0;i<current_table->descendant_classes.size();i++){
        if(bfs_search_and_add(current_table->descendant_classes[i], class_name, descendant_table)){
            return 1;
        }
    }   
    return 0;
}

void check_and_inherit_class(string class_name, symbol_table* descendant_table){

    for(int i=0;i<global_table->children.size();i++){
        if(global_table->children[i]->table_type == "class"){

            if(bfs_search_and_add(global_table->children[i], class_name, descendant_table)){
                return;
            }
        }
    }
    cerr<<"Error at line"<<yylineno<<": Class " << class_name << " not found";
    exit(1);
}


//we have to check in the current scope and all parent scopes
table_entry* symbol_table_lookup(string name){
    symbol_table *current_table = table_stack.top();
    while(current_table != NULL){
        for(int i = 0; i < current_table->entries.size(); i++){
            if(current_table->entries[i]->name == name){
                return current_table->entries[i];
            }
        }
        current_table = current_table->parent;
    }
    return NULL;

}

void add_scope_entries_to_parent(){

    //add the entries to the parent if it doesnt exist. If it exists, do not add.

    symbol_table* current_table = table_stack.top();
    symbol_table* parent_table = current_table->parent;


    for(int j=0;j<current_table->entries.size();j++){
        int found = 0;
        
        for(int i=0;i<parent_table->entries.size();i++){
                
                if(parent_table->entries[i]->name == current_table->entries[j]->name){
                    found = 1;
                }
            
        }

        if(found){
            continue;
        }else{
            parent_table->entries.push_back(current_table->entries[j]);
        }
    }
            
}


symbol_table* get_obj_class(string class_name){
    
    //cerr<<"class_name"<<class_name<<endl;
    for(int i=0;i<global_table->children.size();i++){
        if(global_table->children[i]->table_type == "class"){
            if(global_table->children[i]->name == class_name){
                return global_table->children[i];
            }
        }
    }
    return NULL;
}


int calculate_class_obj_type_size(symbol_table* class_table){
    int size = 0;
    while(class_table != NULL){
        for(int i=0;i<class_table->entries.size();i++){
            if(class_table->entries[i]->symbol_table == NULL){
                //add only if first 5 chars are self.
                if(class_table->entries[i]->name.substr(0,5) == "self."){
                    size += type_size[class_table->entries[i]->type];
                }
            }
        }
        class_table = class_table->parent;
    }

    return size;
}

int calculate_class_type_size(symbol_table* class_table){
    // do for all inheritence levels
    int size = 0;
    symbol_table* current_table = class_table;
    while(current_table != NULL){
        for(int i=0;i<current_table->entries.size();i++){
            if(current_table->entries[i]->symbol_table == NULL){
                size += type_size[current_table->entries[i]->type];
            }
        }
        current_table = current_table->parent;
    }
    return size;

}


int check_type(string t1, string t2){

    if(t1 == "ANY" || t2 == "ANY"){
        return 1;
    }
    if(t1 == "STR" && t2 == "STRING"){
        return 1;
    }
    if(t1 == "STR" && t2 == "STR"){
        return 1;
    }

    if(t1==t2){
        return 1;
    }


    //INT, FLOAT == NUMBER
    if((t1 == "INT") && (t2 == "NUMBER")){
        return 1;
    }

    //search in classes for the type
    symbol_table* class_table_1 = get_obj_class(t1);
    symbol_table* class_table_2 = get_obj_class(t2);

    //check if t2 is a descendant of t1
    if(class_table_1 != NULL && class_table_2 != NULL){
        if(bfs_search(class_table_1, t2)){
            return 1;
        }
    }

    


    return 0;
}
int verify_formal_and_actual_arguements(string name, vector<table_entry*> entries){

    
    //search for the function first starting from nearest scope and then global scope
    symbol_table *current_table = table_stack.top();
    int function_found = 0;

    if(name == "range"){
        if(entries.size() == 1){
            if(entries[0]->type == "INT"){
                return 1;
            }else if(entries[0]->type == "NUMBER"){
                return 1;
            }else{
                cerr<<"Error at line"<<yylineno<<": Type of parameter in function "<<name<<" do not match"<<endl;
                exit(1);
            }
        }else if(entries.size() == 2){
            if(entries[0]->type == "INT" && entries[1]->type == "INT"){
                return 1;
            }else if(entries[0]->type == "NUMBER" && entries[1]->type == "NUMBER"){
                return 1;
            }else if(entries[0]->type == "NUMBER" && entries[1]->type == "INT"){
                return 1;
            }else if(entries[0]->type == "INT" && entries[1]->type == "NUMBER"){
                return 1;
            }else{
                cerr<<"Error at line"<<yylineno<<": Type of parameter in function "<<name<<" do not match"<<endl;
                // cerr<<"entries type "<<entries[0]->type<<" "<<entries[1]->type<<endl;
                exit(1);
            }
        }

        cerr<<"Error at line number"<< yylineno<<"Number of parameters in function "<<name<<" do not match"<<endl;
    }

    //check if class, then check the init of that class
    symbol_table* class_table = get_obj_class(name);

    if(class_table != NULL){
        //check if the init function exists
        for(int i=0;i<class_table->entries.size();i++){
            //ignore init function

            if(class_table->entries[i]->symbol_table!=NULL && class_table->entries[i]->name == "__init__"){
                //check the parameters
                function_found = 1;
                if(entries.size()==0 && class_table->entries[i]->symbol_table->parameters.size()==1){
                    return 1;
                }
                
                if(class_table->entries[i]->symbol_table->parameters.size()-1 != entries.size()){
                    cerr<<"Error at line number"<<yylineno<<": Number of parameters in function "<<name<<" do not match"<<endl;
                    exit(1);
                }

                for(int j=0;j<class_table->entries[i]->symbol_table->parameters.size()-1;j++){
                    // if(!check_type(class_table->entries[i]->symbol_table->parameters[j+1]->type, entries[j]->type)){
                    //     cerr<<"Error at line number"<<yylineno<<": Type of parameter "<<class_table->entries[i]->symbol_table->parameters[j]->name<<" in function "<<name<<" do not match"<<endl;
                    //     exit(1);
                    // }
                }
                return 1;
            }
        }
    }
    //check for the case of recursive functions
    if(current_table->table_type == "function" && current_table->name == name){
        function_found = 1;
        if(entries.size()==0 && current_table->parameters.size()==0){
            return 1;
        }
        
        if(current_table->parameters.size() != entries.size()){
            cerr<<"Error at line number"<<yylineno<<": Number of parameters in function "<<name<<" do not match"<<endl;
            exit(1);
        }

        for(int j=0;j<current_table->parameters.size();j++){
            // if(!check_type(current_table->parameters[j]->type, entries[j]->type)){
            //     cerr<<"Error at line number"<<yylineno<<": Type of parameter "<<current_table->parameters[j]->name<<" in function "<<name<<" do not match"<<endl;
            //     exit(1);
            // }
        }
        return 1;
    }
    while(current_table != NULL && !function_found){
        
        for(int i = 0; i < current_table->entries.size(); i++){
            
            if(current_table->entries[i]->symbol_table!=NULL && current_table->entries[i]->name == name && current_table->entries[i]->symbol_table->table_type == "function"){
                
                //check the parameters
                function_found = 1;

                if(entries.size()==0 && current_table->entries[i]->symbol_table->parameters.size()==0){
                    return 1;
                }
                
                if(current_table->entries[i]->symbol_table->parameters.size() != entries.size()){
                    cerr<<"Error at line number"<<yylineno<<": Number of parameters in function "<<name<<" do not match"<<endl;
                    exit(1);
                }

                for(int j=0;j<current_table->entries[i]->symbol_table->parameters.size();j++){
                    // if(!check_type(current_table->entries[i]->symbol_table->parameters[j]->type, entries[j]->type)){
                    //     cerr<<"Error at line number"<<yylineno<<": Type of parameter "<<current_table->entries[i]->symbol_table->parameters[j]->name<<" in function "<<name<<" do not match"<<endl;
                    //     exit(1);
                    // }
                }
                return 1;
            }
            if(function_found){
                break;
            }

        }
        current_table = current_table->parent;
    }
    
    if(!function_found){
        cerr<<"Error at line number"<<yylineno<<": Function "<< name <<" not found"<<endl;
        exit(1);
    }
    return 1;
}

table_entry* object_lookup(string name){
    //search for the object in the current scope and all parent scopes
    symbol_table *current_table = table_stack.top();

    while(current_table != NULL){
        for(int i = 0; i < current_table->entries.size(); i++){
            if(current_table->entries[i]->name == name && current_table->entries[i]->symbol_table==NULL){
                
                return current_table->entries[i];
            }
        }
        current_table = current_table->parent;
    }
    return NULL;
}



int verify_formal_and_actual_arguements_dot(string class_name, string function_name, vector<table_entry*> entries){
    
    int function_found = 0;

    table_entry* object_entry = object_lookup(class_name); //in case it may be an object
    
    // cerr<<object_entry->type<<class_name<<endl;
    if(object_entry != NULL){
        string class_e = object_entry->type;
        symbol_table* class_table = get_obj_class(class_e);

        while(class_table!=NULL){
            // cerr<<"class_table name"<<class_table->name<<endl;

            for(int i=0;i<class_table->entries.size();i++){
                // cerr<<"class_table name"<<class_table->entries[i]->name<<endl;
                if(class_table->entries[i]->symbol_table!=NULL && class_table->entries[i]->symbol_table->table_type=="function" && class_table->entries[i]->name == function_name){
                    //check the parameters
                    function_found = 1;
                    //here we also have to check for the case of self i.e the first parameter must be self
                    if(entries.size()==0 && class_table->entries[i]->symbol_table->parameters.size()==1){
                        return 1;
                    }

                    if(class_table->entries[i]->symbol_table->parameters.size()-1 != entries.size()){
                        cerr<<"Error at line number"<<yylineno<<": Number of parameters in function "<<function_name<<" do not match"<<endl;
                        exit(1);
                    }

                    for(int j=0;j<class_table->entries[i]->symbol_table->parameters.size()-1;j++){
                        
                        // if(!check_type(class_table->entries[i]->symbol_table->parameters[j+1]->type, entries[j]->type)){
                            
                        //     cerr<<"Error at line number"<<yylineno<<": Type of parameter "<<class_table->entries[i]->symbol_table->parameters[j+1]->name<<" in function "<<function_name<<" do not match"<<endl;
                        //     exit(1);
                        // }
                    }
                    

                    //check if the first entry of the formal is self
                    if(class_table->entries[i]->symbol_table->parameters[0]->name != "self"){
                        cerr<<"Error at line number"<<yylineno<<": First parameter of function "<<function_name<<" must be self"<<endl;
                        exit(1);
                    }
                    return 1;
                }
            }

            class_table = class_table->parent;
            // cerr<<"class_table name"<<class_table->name<<endl;
            
        }

        
       
    }


    symbol_table* class_table = get_obj_class(class_name);
    // cerr<<"class_table name"<<class_name<<endl;
    if(class_table == NULL){
        cerr<<"Error at line number"<<yylineno<<": Class or object "<<class_name<<" not found"<<endl;
        exit(1);
    }
    //now loop through the entries of the class

    for(int i=0;i<class_table->entries.size();i++){
        if(class_table->entries[i]->symbol_table!=NULL && class_table->entries[i]->symbol_table->table_type=="function" && class_table->entries[i]->name == function_name){


            function_found = 1;
            if(entries.size()==0 && class_table->entries[i]->symbol_table->parameters.size()==0){
                    return 1;
            }
            if(class_table->entries[i]->symbol_table->parameters.size() != entries.size()){
                cerr<<"Error at line number"<<yylineno<<": Number of parameters in function "<<function_name<<" do not match"<<endl;
                exit(1);
            }
            for(int j=0;j<class_table->entries[i]->symbol_table->parameters.size();j++){
                //do not check if entry is self
                
                if(entries[j]->name == "self" && class_table->entries[i]->symbol_table->parameters[j]->name == "self"){
                    continue;
                }

                // if(!check_type(class_table->entries[i]->symbol_table->parameters[j]->type, entries[j]->type)){
                //     cerr<<"Error at line number"<<yylineno<<": Type of parameter "<<class_table->entries[i]->symbol_table->parameters[j]->name<<" in function "<<function_name<<" do not match"<<endl;
                //     exit(1);
                // }
            }
            return 1;
        }
    }

    //check if it is an object

    

    if(!function_found){
        cerr<<"Error at line number"<<yylineno<<": Function "<<function_name<<" not found in class "<<class_name<<endl;
        exit(1);
    }
    return 1;

}



//add common global functions like print and len


void add_common_funcs(){
    //add print function
    vector<table_entry*> parameters;
    table_entry *entry = create_table_entry("print", "ANY", NULL, -1, "FUNCDEF", -1);
    parameters.push_back(entry);
    symbol_table *print_table = create_symbol_table_function("print", parameters, "VOID", "function", 0);
    table_entry *entry1 = create_table_entry("print", "VOID", print_table, -1, "FUNCDEF", -1);
    add_entry_to_entries(global_table->entries, entry1);
    add_to_global_table(print_table);

    //add len function
    vector<table_entry*> parameters1;
    table_entry *entry2 = create_table_entry("len", "ANY", NULL, -1, "FUNCDEF", -1);
    parameters1.push_back(entry2);
    symbol_table *len_table = create_symbol_table_function("len", parameters1, "INT", "function", 0);
    table_entry *entry3 = create_table_entry("len", "INT", len_table, -1, "FUNCDEF", -1);   
    add_entry_to_entries(global_table->entries, entry3);
    add_to_global_table(len_table);

    //add range function 
    vector<table_entry*> parameters2;
    table_entry *entry4 = create_table_entry("range", "ANY", NULL, -1, "FUNCDEF", -1);
    parameters2.push_back(entry4);
    symbol_table *range_table = create_symbol_table_function("range", parameters2, "NUMBER", "function", 0);
    table_entry *entry5 = create_table_entry("range", "INT", range_table, -1, "FUNCDEF", -1);
    add_entry_to_entries(global_table->entries, entry5);
    add_to_global_table(range_table);

}



int check_augassign_merge(string t1, string t2){
    if(t1 == "INT" && t2 == "INT"){
        return 1;
    }
    // }else if(t1 == "FLOAT" && t2 == "FLOAT"){
    //     return 1;
    // }else if(t1 == "FLOAT" && t2 == "INT"){
    //     return 1;
    // }else if(t1 == "INT" && t2 == "FLOAT"){
    //     return 1;
    // }

    if(t1 == "NUMBER" && (t2 == "INT"||t2 == "FLOAT")){
        return 1;
    }else if(t2 == "NUMBER" && (t1 == "INT"||t1 == "FLOAT")){
        return 1;
    }

    cerr <<"Error at line number"<<yylineno<<": Type mismatch in augassign expression\n"<<endl;
    exit(1);

}

int check_equal_to_expr(string t1, string t2){
    if(t2 == "NOTDEFINED"){
        return 1;
    }

    if(t1 == "BOOL" && t2 == "BOOL"){
        return 1;
    }
    if(t1 == "INT" && t2 == "INT"){
        return 1;
    }else if(t1 == "FLOAT" && t2 == "FLOAT"){
        return 1;
    }

    if(t1 == "NUMBER" && (t2 == "INT"||t2 == "FLOAT")){
        return 1;
    }

    if(t1 == "STR" && t2 == "STR"){
        return 1;
    }
    if(t1 == "STR" && t2 == "STRING"){
        return 1;
    }

    if(t1 == "STRING" && t2 == "STR"){
        return 0;
    }
   
    cerr<<"Error at line number"<<yylineno<<": Type mismatch in equal to expression\n"<<endl;
    exit(1);
}

int check_not_test_type(string t1){
    if(t1 == "NOTDEFINED"){
        return 1;
    }
    if(t1 == "FLOAT"){
        return 1;
    }
    if(t1 == "INT"){
        return 1;
    }
    if(t1 == "NUMBER"){
        return 1;
    }
    if(t1 == "BOOL"){
        return 1;
    }
    cerr<<"Error  at line number"<<yylineno<<": Type mismatch in not test expression\n"<<t1<<endl;
    exit(1);
}

string merge_comp_types(string t1, string t2){
    if(t1=="INT" && t2 == "INT"){
        return "BOOL";
    }
    if(t1=="INT" && t2 == "FLOAT"){
        return "BOOL";
    }
    if(t1=="NUMBER" && (t2 == "INT"||t2 == "FLOAT")){
        return "BOOL";
    }
    if(t2=="NUMBER" && (t1 == "INT"||t1 == "FLOAT")){
        return "BOOL";
    }
    if(t1 == "STR" && t2 == "STR"){
        return "BOOL";
    }
    if(t1 == "STR" && t2 == "STRING"){
        return "BOOL";
    }

    if(t1 == "STRING" && t2 == "STR"){
        return "BOOL";
    }
    if(t1 == "STRING" && t2 == "STRING"){
        return "BOOL";
    }
    cerr <<"Error at line number"<<yylineno<<": Type mismatch in comparison expression\n"<<t1<<endl;
    exit(1);
    
}

string merge_and_expr_types(string t1, string t2){
    if(t1=="NOTDEFINED"){
        return t2;
    }
    if(t1=="NOTDEFINED"){
        return t2;
    }
    if(t1 == "INT" && t2 == "INT"){
        return "INT";
    }
    if(t1 == "INT" && t2 == "NUMBER"){
        return "INT";
    }
    if(t1 == "NUMBER" && t2 == "INT"){
        return "INT";
    }
    if(t1 == "NUMBER" && t2 == "NUMBER"){
        return "INT";
    }
    cerr <<"Error at line number"<<yylineno<<": Type mismatch in and expression\n"<<endl;
    exit(1);
}

string merge_arith_expr_types(string t1, string t2){
    if(t1 == "NOTDEFINED"){
        return t2;
    }

    if(t2 == "NOTDEFINED"){
        return t1;
    }


    if(t1 == "INT" && t2 == "INT"){
        return "INT";
    }
    // else if(t1 == "FLOAT" && t2 == "FLOAT"){
    //     return "FLOAT";
    // }else if(t1 == "FLOAT" && t2 == "INT"){
    //     return "FLOAT";
    // }else if(t1 == "INT" && t2 == "FLOAT"){
    //     return "FLOAT";
    // }


    if(t1 == "NUMBER" && (t2 == "INT"||t2 == "FLOAT")){
        return t2;
    }else if(t2 == "NUMBER" && (t1 == "INT"||t1 == "FLOAT")){
        return t1;
    }

    if(t1=="NUMBER" && t2 == "NUMBER"){
        return "NUMBER";
    }
    cerr <<"Error at line number"<<yylineno<<": Type mismatch in arithmetic expression\n"<<endl;
    exit(1);
}

string merge_factor(string t1){
    if(t1 == "INT"){
        return "INT";
    }
    // if(t1 == "FLOAT"){
    //     return "FLOAT";
    // }

    if(t1 == "NUMBER"){
        return "NUMBER";
    }

    
    cerr<<"Error at line number"<<yylineno<<": Type mismatch in factor expression\n"<<endl;
    exit(1);
    
}

string merge_power(string t1, string t2){
    if(t1 == "INT" && t2 == "INT"){
        return "INT";
    }
    // else if(t1 == "FLOAT" && t2 == "FLOAT"){
    //     return "FLOAT";
    // }else if(t1 == "FLOAT" && t2 == "INT"){
    //     return "FLOAT";
    // }else if(t1 == "INT" && t2 == "FLOAT"){
    //     return "FLOAT";
    // }

    if(t1 == "NUMBER" && (t2 == "INT"||t2 == "FLOAT")){
        return "INT";
    }else if(t2 == "NUMBER" && (t1 == "INT"||t1 == "FLOAT")){
        return "INT";
    }
    else if(t1 == "NUMBER" && t2 == "NUMBER"){
        return "INT";
    }

    cerr <<"Error at line number"<<yylineno<<": Type mismatch in power expression\n"<<endl;
    exit(1);

}
string merge_and_test_types(string t1, string t2){

    if(t1 == "NOTDEFINED"){
        return t2;
    }

    if(t1 == "BOOL" && t2=="BOOL"){
        return "BOOL";

    }


    if(t1 == "INT" && t2 == "INT"){
        return "BOOL";
    }
    if(t1 == "INT" && t2 == "NUMBER"){
        return "BOOL";
    }
    if(t1 == "NUMBER" && t2 == "INT"){
        return "BOOL";
    }
    if(t1 == "NUMBER" && t2 == "NUMBER"){
        return "BOOL";
    }

    cerr<<"Error at line number"<<yylineno<<": Type mismatch in and test expression\n"<<endl;    
    exit(1);
}
string merge_not_test_types(string t1){
    // if(t1 == "FLOAT"){
    //     return "BOOL";
    // }
    
    if(t1 == "INT"){
        return "BOOL";
    }
    if(t1 == "NUMBER"){
        return "BOOL";
    }
    if(t1 == "BOOL"){
        return "BOOL";
    }
    cerr<<"Error at line number"<<yylineno<<": Type mismatch in not test expression\n"<<endl;
    exit(1);

}

string merge_term(string t1, string t2){
   
    // if(t1 == "NOTDEFINED" && (t2!="STR" && t2!="STRING")){
    //     return t2;
    // }

    // if(t2 == "NOTDEFINED" && (t1!="STR" && t1!="STRING")){
    //     return t1;
    // }
    if(t1 == "NOTDEFINED"){
        return t2;
    }
    if(t2 == "NOTDEFINED"){
        return t1;
    }
    

    if(t1 == "INT" && t2 == "INT"){
        return "INT";
    }
    // else if(t1 == "FLOAT" && t2 == "FLOAT"){
    //     return "FLOAT";
    // }else if(t1 == "FLOAT" && t2 == "INT"){
    //     return "FLOAT";
    // }else if(t1 == "INT" && t2 == "FLOAT"){
    //     return "FLOAT";
    // }

    if(t1 == "NUMBER" && (t2 == "INT"||t2 == "FLOAT")){
        return "INT";
    }else if(t2 == "NUMBER" && (t1 == "INT"||t1 == "FLOAT")){
        return "INT";
    }

    if(t1 == "NUMBER" && t2 == "NUMBER"){
        return "INT";
    }
    cerr<<t1<<t2<<endl;
    cerr <<"Error at line number"<<yylineno<<": Type mismatch in term expression\n"<<endl;
    exit(1);

}

string merge_shift_expr_types(string t1, string t2){
    if(t1 == "INT" && t2 == "INT"){
        return "INT";
    }
    if(t1=="NOTDEFINED"){
        return t2;
    }
    if(t1 == "INT" && t2=="NUMBER"){
        return "INT";
    }
    if(t1 == "NUMBER" && t2=="INT"){
        return "INT";
    }
    cerr <<"Error at line number"<<yylineno<<": Type mismatch in shift expression\n"<<endl;
    exit(1);

}

string get_dot_type(string obj_name, string attr_name){
    table_entry* object_entry = object_lookup(obj_name);
    cerr<<"object_entry"<<obj_name<<endl;
    
    if(object_entry == NULL){
        cerr<<"Error at line number"<<yylineno<<": Object "<<obj_name<<" not found in the current scope"<<endl;
        exit(1);
    }
    string class_name = object_entry->type;
    symbol_table* class_table = get_obj_class(class_name);
    while(class_table!=NULL){
        for(int i=0;i<class_table->entries.size();i++){
            cerr<<"class_table entries"<<class_table->entries[i]->name<<class_table->entries[i]->type<<endl;
            if(class_table->entries[i]->name == attr_name){
                return class_table->entries[i]->type;
            }
        }
        class_table = class_table->parent;
    }
    cerr<<"Error at line number"<<yylineno<<": Attribute "<<attr_name<<" not found in class "<<class_name<<endl;
    // return "STRING";
    exit(1);
}

string get_return_type(string name){
    
    symbol_table *current_table = table_stack.top();
    //check for recursion
    if(current_table->name == name && current_table->table_type == "function"){
        return current_table->return_type;
    }
    while(current_table != NULL){
        for(int i = 0; i < current_table->entries.size(); i++){
            if(current_table->entries[i]->name == name && current_table->entries[i]->symbol_table->table_type == "function"){
                return current_table->entries[i]->type;
            }
        }
        current_table = current_table->parent;
    }
    
    //check if it is a class
    symbol_table* class_table = get_obj_class(name);
    // cerr<<"class_table 1"<<class_table->name <<endl;
    if(class_table != NULL){
        //check if the init function exists
        for(int i=0;i<class_table->entries.size();i++){
            //ignore init function
            if(class_table->entries[i]->symbol_table!=NULL && class_table->entries[i]->name == "__init__"){
                return class_table->entries[i]->symbol_table->name;
            }
        }
    }

    cerr<<"Error at line number"<<yylineno<<": function " << name << " not found in the current scope"<<endl;
    exit(1);

}

string get_return_type_dot(string class_name, string function_name){
   
    //incase called from object
    // cerr<<"getting return type"<<class_name<<" "<<function_name<<endl;
    table_entry* object_entry = object_lookup(class_name); //in case it may be an object
    if(object_entry != NULL){
        string class_e = object_entry->type;
        symbol_table* class_table = get_obj_class(class_e);
        while(class_table!=NULL){
            for(int i=0;i<class_table->entries.size();i++){
                if(class_table->entries[i]->symbol_table!=NULL && class_table->entries[i]->name == function_name){
                    return class_table->entries[i]->symbol_table->return_type;
                }
            }
            class_table = class_table->parent;
        }
    }
    // cerr<<"class_name getting"<<class_name<<endl;

    symbol_table* class_table = get_obj_class(class_name);
    for(int i=0;i<class_table->entries.size();i++){
        if(class_table->entries[i]->symbol_table!=NULL && class_table->entries[i]->symbol_table->table_type=="function" && class_table->entries[i]->name == function_name){
            // cerr<<"return type"<<class_table->entries[i]->symbol_table->return_type<<endl;
            return class_table->entries[i]->symbol_table->return_type;
        }
    }
    cerr<<"Error at line number"<<yylineno<<": Function "<<function_name<<" not found in class "<<class_name<<endl;
    exit(1);
}


string get_array_ref_type(string 
name, node_ast* index){
    table_entry* entry = symbol_table_lookup(name);
    if(entry == NULL){
        cerr<<"Error at line number"<<yylineno<<": Variable "<<name<<" not found in the current scope"<<endl;
        exit(1);
    }

    if(entry->symbol_table != NULL){
        cerr<<"Error at line number"<<yylineno<<": Variable "<<name<<" is a function"<<endl;
        return entry->symbol_table->return_type;
    }
    if(entry->is_array){
    
        if(index->name == "NAME"){
            table_entry* index_entry = symbol_table_lookup(index->lexval);
            if(index_entry == NULL){
                cerr<<"Error: Variable "<<index->lexval<<" not found in the current scope"<<endl;
                exit(1);
            }
            if(index_entry->type != "INT"){
                cerr<<"Error: Array index should be of type INT"<<endl;
                exit(1);
            }
        }else if(index->name == "NUMBER"){
            // if(stoi(index->lexval) >= entry->array_size){
            //     cerr<<"Error: Array index out of bounds"<<endl;
            //     exit(1);
            // }
        }
            return entry->array_type;
    }

    cerr<<"Error at line number"<<yylineno<<": Variable "<<name<<" is not an array"<<endl;
    exit(1);

}



table_entry* func_table_entry(string name){

    symbol_table *current_table = table_stack.top();
    //check for recursion
    if(current_table->name == name && current_table->table_type == "function"){
        //get its entry in its parent
        // symbol_table* parent_table = current_table->parent;
        // for(int i=0;i<parent_table->entries.size();i++){
        //     if(parent_table->entries[i]->name == name){
        //         return parent_table->entries[i];
        //     }
        // }
    }

    while(current_table != NULL){
        for(int i = 0; i < current_table->entries.size(); i++){
            if(current_table->entries[i]->name == name && current_table->entries[i]->symbol_table->table_type == "function"){
                return current_table->entries[i];
            }
        }
        current_table = current_table->parent;
    }


    //check if it is a class
    symbol_table* class_table = get_obj_class(name);
    if(class_table != NULL){
        //check if the init function exists
        for(int i=0;i<class_table->entries.size();i++){

            if(class_table->entries[i]->symbol_table!=NULL && class_table->entries[i]->name == "__init__"){
                
                return class_table->entries[i];
            }
        }
    }

    cerr<<"Error at line number"<<yylineno<<": function " << name << " not found in the current scope"<<endl;
    exit(1);


}

table_entry* func_table_entry_dot(string class_name, string function_name){
    
    //incase called from object
    int function_found = 0;
    table_entry* object_entry = object_lookup(class_name); //in case it may be an object
    // cerr<<"object_entry"<<object_entry->type<<endl;
    
    if(object_entry != NULL){
        string class_e = object_entry->type;
        symbol_table* class_table = get_obj_class(class_e);
        // cerr<<"class_table name"<<class_table->name;
        while(class_table!=NULL){
            // cerr<<"class_table name"<<class_table->name<<endl;

            for(int i=0;i<class_table->entries.size();i++){
                // cerr<<"class_table name"<<class_table->entries[i]->name<<endl;
                if(class_table->entries[i]->symbol_table!=NULL && class_table->entries[i]->name == function_name){
                    return class_table->entries[i];
                }
            }
            class_table = class_table->parent;
        }
        
    }

    // cerr<<"class_name getting"<<class_name<<endl;
    symbol_table* class_table = get_obj_class(class_name);
    
    if(class_table == NULL){
        cerr<<"Error at line number"<<yylineno<<": Class "<<class_name<<" not found"<<endl;
        exit(1);
    }
    
    for(int i=0;i<class_table->entries.size();i++){
        if(class_table->entries[i]->symbol_table!=NULL && class_table->entries[i]->symbol_table->table_type=="function" && class_table->entries[i]->name == function_name){
            //check the parameters
            function_found = 1;
            return class_table->entries[i];
        }
    }

    if(!function_found){
        cerr<<"Error at line number"<<yylineno<<": Function "<<function_name<<" not found in class "<<class_name<<endl;
        exit(1);
    }

    cerr<<"Error at line number"<<yylineno<<": Function "<<function_name<<" not found in class "<<class_name<<endl;
    exit(1);
    return NULL;
}

void verify_return_type(string type, string return_type){

    if(return_type == "VOID"){
        if(return_type != "VOID"){
            cerr<<"Error at line number"<<yylineno<<": Function should not return a value"<<endl;
            exit(1);
        }
    }

    if(type == "NUMBER" && (return_type == "INT" || return_type == "FLOAT")){
        return;
    }


    if(type == "INT" && return_type == "INT"){
        return;
    }

    // if(type == "FLOAT" && return_type == "FLOAT"){
    //     return;
    // }
    if(type == "BOOL" && (return_type =="INT"||return_type == "NUMBER")){
        return;
    }
    if(type == "STR" && return_type == "STR"){
        return;
    }

    if(type == "STR" && return_type == "STRING"){
        return;
    }

    if(type == "STRING" && return_type == "STRING"){
        return;
    }

    if(type == "STRING" && return_type == "STR"){
        return;
    }

    if(type == "BOOL" && return_type == "BOOL"){
        return;
    }
    
    cerr<<"Error: Return type mismatch"<<endl;
    exit(1);
    
}


string get_return_type_obj_variable(string class_name, string variable){

    symbol_table* class_table = get_obj_class(class_name);
    while(class_table != NULL){
        for(int i=0;i<class_table->entries.size();i++){
            if(class_table->entries[i]->symbol_table == NULL){
                if("self."+ variable == class_table->entries[i]->name){
                    return class_table->entries[i]->type;
                    
                }
            }
        }
        class_table = class_table->parent;
    }

    return "NOTDEFINED";

}