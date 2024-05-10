#include <iostream>
#include <string>
#include<stdlib.h>
#include <vector>
#include "symbol_table.cpp"
extern int yylineno;

using namespace std;

int error_line = 0;
int verbose = 0;
string ast_type = "";
string dotfile_path = "";
extern char* input_file;
static int temp_count = 0;
static int label_count = 0;

int print_string = 0;


struct threeac* create_3AC(string op, string arg1, string arg2, string result){
   
    // struct threeac* tac = (struct threeac*)malloc(sizeof(struct threeac));
    struct threeac* tac = new struct threeac;
    tac->op = op;
    tac->arg1 = arg1;
    tac->arg2 = arg2;
    tac->result = result;
    return tac;
}

void append_3AC(node_ast* parent, node_ast* child){
    // cerr<<"Appending 3AC for "<<child->lexval<<endl;
    for(int i = 0; i < child->code.size(); i++){
        parent->code.push_back(child->code[i]);
    }
    child->code.clear();

}
void create_new_temp(node_ast* node){
    node->addr = "t" + to_string(temp_count);
    temp_count++;
}

struct label* create_new_label(){
    label_count++;
    // struct label* label =  (struct label*)malloc(sizeof(struct label));
    struct label* label = new struct label;
    label->name = "L" + to_string(label_count);
    return label;
}

struct threeac* create_3AC_label(struct label* label){
    // struct threeac* tac = (struct threeac*)malloc(sizeof(struct threeac));
    struct threeac* tac = new struct threeac;
    tac->label = label;
    tac->triplet_code = label->name + ":\n";
    return tac;
}


struct label* create_label_name(string name){
    // struct label* label =  (struct label*)malloc(sizeof(struct label));
    struct label* label = new struct label;
    label->name = name;
    return label;

}

// int get_activation_record_size(table_entry* entry){
//     //allocate the space for return address.
//     int size = 8; //64 bit return address, x86_64

//     size+=8; //space for the rbp register



// }

void create_3AC_code_expression(threeac* tac){

    tac->triplet_code = "\t\t" + tac->result + " = " + tac->arg1 + " " + tac->op + " " + tac->arg2 + ";\n";
}

void create_3AC_code_placeholders(threeac* tac){
    tac->triplet_code = "\t" + tac->op+";\n";
}

void create_3AC_pop_push(threeac* tac){
    tac->triplet_code = "\t\t" + tac->op + " " + tac->result + ";\n";
}

void create_3AC_call(threeac* tac){
    tac->triplet_code = "\t\t" + tac->op + " " + tac->arg1 + " " + tac->arg2 + ";\n";
}

void create_3AC_code_equal(threeac* tac){
    tac->triplet_code = "\t\t" + tac->result + " = " + tac->arg1 + ";\n";
}


void create_3AC_goto(threeac* tac){
    tac->triplet_code = "\t\t" + tac->op + " " + tac->result + ";\n";
}

void create_3AC_if(threeac* tac, string condition){
    tac->triplet_code = "\t\t" + tac->op+ " " + tac->arg1 + " goto " + tac->result + ";\n";
    tac->is_conditional = true;
    tac->condition = condition;
    
}

void create_3AC_code_array_ref(threeac* tac){
    tac->triplet_code = "\t\t" + tac->result + " = " + tac->arg1 + " + " + tac->arg2 + ";\n";
}

int get_object_offset(string name, symbol_table* class_table){
    int offset = 0;
    while(class_table != NULL){
        for(int i=0;i<class_table->entries.size();i++){
            if(class_table->entries[i]->symbol_table == NULL){
                //add only if first 5 chars are self.
                if("self."+name == class_table->entries[i]->name){
                    return offset;
                }
                else if(class_table->entries[i]->name.substr(0,5) == "self."){
                    offset += type_size[class_table->entries[i]->type];
                }
            }
        }
        class_table = class_table->parent;
    }

    cerr<<"variable not found"<<endl;
    exit(1);
}
void create_3AC_code_data(threeac* tac){
    tac->triplet_code = "\t" + tac->op + " " + tac->arg1+";\n";
}
void add_data_section(node_ast* node){
    //add the data section
    node->code.push_back(create_3AC(".data", "", "", ""));
    create_3AC_code_placeholders(node->code[node->code.size()-1]);
    for(auto it = label_to_string.begin(); it != label_to_string.end(); it++){
        node->code.push_back(create_3AC_label(create_label_name(it->first)));
        
        node->code.push_back(create_3AC(".string", it->second, "", ""));
        create_3AC_code_data(node->code[node->code.size()-1]);

        // create_3AC_code_placeholders(node->code[node->code.size()-1]);

        //label_to_string is a map, we need to print index by index

    }

}



int find_temp(string temp){
    //search in string_temps vector
    // cerr<<"Searching for "<<temp<<endl;
    for(int i=0;i<string_temps.size();i++){
        if(string_temps[i] == temp){
            return 1;
        }
    }
    return 0;
}

void generate_3AC(node_ast* node){
    for (int i = 0; i < node->children.size(); i++){
        generate_3AC(node->children[i]);
    }
    // cerr<<"Generating 3AC for "<<node->name<<endl;
    if(node->name=="NAME" || node->name =="NUMBER"||node->name =="STRING"){
        
        //if we have a string variable type...we need to give it the string label and push it back
        //to string temps
        
        if(node->name == "STRING"){
            create_new_temp(node);

            //push back to string temps
            string_temps.push_back(node->addr);
            string label = string_to_label[node->lexval];
            node->code.push_back(create_3AC("=", label, "", node->addr));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "STR_ASSIGN";
            return;
        }
        node->addr = node->lexval;
        node->code = vector<threeac*>();
        

        if(node->children.size()==1 && node->children[0]->name =="ATOM_EXPR"){
            //the name of an array may be obj.
            // array initialisation for obj.array
            
            if(node->is_dot){
                symbol_table* class_table = get_obj_class(node->obj_class_name);
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                int offset=get_object_offset(node->dot_name, class_table);
                node->code.push_back(create_3AC("+", node->dot_obj, to_string(offset), temp1));
                create_3AC_code_expression(node->code[node->code.size()-1]);
                node->addr = "*"+temp1;
                
            }
            
        }
        else if(node->is_dot){
            // cerr<<"Dot found"<<node->lexval<<endl;
            // cerr<<"Type: "<<node->type<<endl;
            symbol_table* class_table = get_obj_class(node->obj_class_name);
            
            // cerr<<"Class table: "<<class_table->name<<endl;

            //get the offset of the variable
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            
            int offset=get_object_offset(node->dot_name, class_table);
            // cerr<<"Offset: "<<offset<<endl;
            node->code.push_back(create_3AC("+", node->dot_obj, to_string(offset), temp1));
            // cerr<<"Creating 3AC for "<<node->dot_obj<<" + "<<to_string(offset)<<" = "<<temp1<<endl;
            create_3AC_code_expression(node->code[node->code.size()-1]);
           
            // create_new_temp(node);
            // node->code.push_back(create_3AC("=", "*"+temp1, "", node->addr));
            // create_3AC_code_equal(node->code[node->code.size()-1]);
            // node->code[node->code.size()-1]->equal_type = "LOAD";
            
            node->addr = "*"+temp1;
            // cerr<<"New node addr: "<<node->addr<<endl;
            
            if(node->type == "STRING"|| node->type == "STR"){
                // cerr<<node->addr<<"is a string"<<endl;
                string_temps.push_back(node->addr);
                
            }
            // cerr<<"Dot found"<<node->addr<<endl;
        }
        else if(node->label_true && node->label_false){
            node->code.push_back(create_3AC("if", node->addr , "goto" , node->label_true->name));

            create_3AC_if(node->code[node->code.size()-1], "NOT_COMP");
            node->code.push_back(create_3AC("goto", "", "", node->label_false->name));
            create_3AC_goto(node->code[node->code.size()-1]);
        }
        else if(node->type == "STRING"|| node->type == "STR"){
            //in this case we need to be able to identify this name as a string
            //and assign it the label

            //push back to string temps
            // cerr<<node->lexval<<"is a string"<<endl;
            string_temps.push_back(node->addr);
            
        }

    }else if(node->name == "TFPDEF"){
        
        node->code.push_back(create_3AC("pop_param", "", "", node->children[0]->addr));
        create_3AC_pop_push(node->code[node->code.size()-1]);  

 
    }else if(node->name == "TESTLIST_COMP"){
        
        if(node->arr_arg == true){
            string temp_size ="t" + to_string(temp_count);
            temp_count++;

            create_new_temp(node);
            // node->code.push_back(create_3AC("=",to_string(type_size[node->entry->array_type]*node->entry->array_size) ,"" ,temp_size));
            // node->code[node->code.size()-1]->equal_type = "ASSIGN";
            // create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code.push_back(create_3AC("param", "", "", to_string(type_size[node->entry->array_type]*node->entry->array_size)));
            create_3AC_pop_push(node->code[node->code.size()-1]);

            node->code.push_back(create_3AC("call", "memalloc", "1",""));
            create_3AC_call(node->code[node->code.size()-1]);
            node->code.push_back(create_3AC("ret","", "",node->addr ));
            create_3AC_pop_push(node->code[node->code.size()-1]);
            
            for(int i=0;i<node->children.size();i++){
                node->code.push_back(create_3AC("=", node->children[i]->addr ,"" ,"*"+node->addr));
                node->code[node->code.size()-1]->equal_type = "STORE";
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code.push_back(create_3AC("+", node->addr,to_string(type_size[node->entry->array_type]) ,node->addr));
                create_3AC_code_expression(node->code[node->code.size()-1]);
            }
            node->code.push_back(create_3AC("-", node->addr,to_string(type_size[node->entry->array_type]*node->entry->array_size) ,node->addr));
            create_3AC_code_expression(node->code[node->code.size()-1]);
        }else{

            // create_new_temp(node);
            // cerr<<"Error: Testlist_comp not a function argument"<<endl;
            for(int i=0;i<node->children.size();i++){
                append_3AC(node, node->children[i]);
            }
        }
    }

    else if(node->name == "EQUAL"){  
        //This is assignment 
        append_3AC(node, node->children[0]);
        append_3AC(node, node->children[1]);
        

        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            if(find_temp(node->children[1]->addr)){
                //add this temp to the string_temps
                // cerr<<"Adding to string temps"<<node->children[0]->addr<<endl;
                string_temps.push_back(node->children[0]->addr);
            }
            // cerr<<"Performing load operation"<<endl;
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }

        //check if the first is a string temp by searching in vector string_temps
        // cerr<<"1 Searching for "<<node->children[1]->addr<<endl;

        
        node->code.push_back(create_3AC("=", node->children[1]->addr, "", node->children[0]->addr));
        // for last element of node->code
        create_3AC_code_equal(node->code[node->code.size()-1]);
        // node->code[node->code.size()-1]->equal_type = "ASSIGN";
        //check it * is there or not in either side
        if(node->children[0]->addr.substr(0,1) == "*"){
            node->code[node->code.size()-1]->equal_type = "STORE";
        }
        // else if(node->children[1]->addr.substr(0,1) == "*"){
        //     node->code[node->code.size()-1]->equal_type = "LOAD";
        // }
        else{
            node->code[node->code.size()-1]->equal_type = "ASSIGN";
        }


    }else if(node->name == "PLUS"){
        if(node->children.size() == 0){
            //continue, handled in factor
        }else{
            append_3AC(node, node->children[0]);
            
            if(node->children[0]->addr.substr(0,1) == "*"){
                //perform the load operation
                // cerr<<"Performing load operation"<<node->children[0]->addr<<endl;
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[0]->addr = temp1;
            }
            append_3AC(node, node->children[1]);
            if(node->children[1]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[1]->addr = temp1;
            }
            create_new_temp(node);
            node->code.push_back(create_3AC("+", node->children[0]->addr, node->children[1]->addr, node->addr));
            create_3AC_code_expression(node->code[node->code.size()-1]);

        }
    }else if(node->name == "DOUBLE_STAR"){
        // cerr<<"Double star"<<endl;
        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        create_new_temp(node);
        node->code.push_back(create_3AC("**", node->children[0]->addr, node->children[1]->addr, node->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);
    }
    else if(node->name == "MINUS"){
        if(node->children.size() == 0){ //unary minus
           //continue, handled in factor

        }else{
            append_3AC(node, node->children[0]);
            if(node->children[0]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[0]->addr = temp1;
            }
            append_3AC(node, node->children[1]);
            if(node->children[1]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[1]->addr = temp1;
            }
            create_new_temp(node);
            node->code.push_back(create_3AC("-", node->children[0]->addr, node->children[1]->addr, node->addr));
            create_3AC_code_expression(node->code[node->code.size()-1]);

        }
    }else if(node->name == "MULT"){
        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        create_new_temp(node);
        node->code.push_back(create_3AC("*", node->children[0]->addr, node->children[1]->addr, node->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);
        
    }else if(node->name == "DIV"){
        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        create_new_temp(node);
        node->code.push_back(create_3AC("/", node->children[0]->addr, node->children[1]->addr, node->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }else if(node->name == "MOD"){
        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        create_new_temp(node);
        node->code.push_back(create_3AC("%", node->children[0]->addr, node->children[1]->addr, node->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);
    }
    else if(node->name == "LSHIFT"){
        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        create_new_temp(node);
        node->code.push_back(create_3AC("<<", node->children[0]->addr, node->children[1]->addr, node->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);
    }else if(node->name == "RSHIFT"){
        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        create_new_temp(node);
        node->code.push_back(create_3AC(">>", node->children[0]->addr, node->children[1]->addr, node->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);
    }else if(node->name =="AND_EXPR"){
        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
        append_3AC(node, node->children[2]);
        if(node->children[2]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[2]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[2]->addr = temp1;
        }
        create_new_temp(node);
        node->code.push_back(create_3AC("&", node->children[0]->addr, node->children[2]->addr, node->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);
    }
    // else if(node->name =="OR_EXPR"){
    //     append_3AC(node, node->children[0]);
    //     append_3AC(node, node->children[2]);
    //     create_new_temp(node);
    //     node->code.push_back(create_3AC("|", node->children[0]->addr, node->children[2]->addr, node->addr));
    //     create_3AC_code_expression(node->code[node->code.size()-1]);
    // }
    else if(node->name =="XOR_EXPR"){
        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
        append_3AC(node, node->children[2]);
        if(node->children[2]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[2]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[2]->addr = temp1;
        }
        create_new_temp(node);
        node->code.push_back(create_3AC("^", node->children[0]->addr, node->children[2]->addr, node->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);
    }else if(node->name == "EXPR" && node->children.size()> 1 && node->children[1]->name == "PIPE"){
        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
        append_3AC(node, node->children[2]);
        if(node->children[2]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[2]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[2]->addr = temp1;
        }
        create_new_temp(node);
        node->code.push_back(create_3AC("|", node->children[0]->addr, node->children[2]->addr, node->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);
    }
    else if(node->name == "FACTOR"){
        //check if the first is PLUS or MINUS
        if(node->children[0]->name == "PLUS"){
            append_3AC(node, node->children[1]);
            if(node->children[1]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[1]->addr = temp1;
            }
            create_new_temp(node);
            node->code.push_back(create_3AC("=", "+"+ node->children[1]->addr,"", node->addr));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "UNARY_PLUS";

        }else if(node->children[0]->name == "MINUS"){
            append_3AC(node, node->children[1]);
            if(node->children[1]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[1]->addr = temp1;
            }
            create_new_temp(node);
            node->code.push_back(create_3AC("=","-" + node->children[1]->addr,"", node->addr));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "UNARY_MINUS";


        }else if(node->children[0]->name == "TILDE"){
            append_3AC(node, node->children[1]);
            if(node->children[1]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[1]->addr = temp1;
            }
            create_new_temp(node);
            node->code.push_back(create_3AC("=","~" + node->children[1]->addr,"", node->addr));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "NOT";
        }
        else{
            append_3AC(node, node->children[0]);
        }   
    }else if(node->name == "NOT_TEST"){
        //children 0 is NOT, children 1 is test
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        create_new_temp(node);
        node->code.push_back(create_3AC("!", node->children[1]->addr, "", node->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }
    else if (node->name =="PLUSEQUAL"){
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }

        node->code.push_back(create_3AC("+", node->children[0]->addr, node->children[1]->addr, node->children[0]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }else if (node->name =="MINUSEQUAL"){
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        node->code.push_back(create_3AC("-", node->children[0]->addr, node->children[1]->addr, node->children[0]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }else if (node->name =="MULEQUAL"){
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        node->code.push_back(create_3AC("*", node->children[0]->addr, node->children[1]->addr, node->children[0]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }else if (node->name =="DIVEQUAL"){
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        node->code.push_back(create_3AC("/", node->children[0]->addr, node->children[1]->addr, node->children[0]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }else if (node->name =="MODEQUAL"){
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        node->code.push_back(create_3AC("%", node->children[0]->addr, node->children[1]->addr, node->children[0]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }else if(node->name == "AMPEREQUAL"){
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        node->code.push_back(create_3AC("&", node->children[0]->addr, node->children[1]->addr, node->children[0]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }else if(node->name == "OREQUAL"){
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        node->code.push_back(create_3AC("|", node->children[0]->addr, node->children[1]->addr, node->children[0]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }else if(node->name == "XOREQUAL"){
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        node->code.push_back(create_3AC("^", node->children[0]->addr, node->children[1]->addr, node->children[0]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }else if(node->name == "LSHIFTEQUAL"){
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        node->code.push_back(create_3AC("<<", node->children[0]->addr, node->children[1]->addr, node->children[0]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }else if(node->name == "RSHIFTEQUAL"){
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        node->code.push_back(create_3AC(">>", node->children[0]->addr, node->children[1]->addr, node->children[0]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }else if(node->name == "FLOORDIV_ASSIGN"){
        append_3AC(node, node->children[1]);
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }
        node->code.push_back(create_3AC("//", node->children[0]->addr, node->children[1]->addr, node->children[0]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);

    }//TODO: Bitwise ops
    else if(node->name == "ATOM_EXPR" && node->children.size()==2 ){
        //this is an array reference, the first element may be a ref

        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
       
        if(node->children[0]->name!="LIST"){
            create_new_temp(node);
            append_3AC(node, node->children[1]);
            
            if(node->children[1]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[1]->addr = temp1;
            }

            string temp_addr ="t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("*", node->children[1]->addr,to_string(type_size[node->type]), temp_addr));
            create_3AC_code_expression(node->code[node->code.size()-1]);
            node->code.push_back(create_3AC("+",  node->children[0]->addr,temp_addr, temp_addr));
            create_3AC_code_expression(node->code[node->code.size()-1]);
            node->addr = "*"+ temp_addr;

        
        }

        if(node->label_true && node->label_false){
            node->code.push_back(create_3AC("if", node->addr , "goto" , node->label_true->name));
            create_3AC_if(node->code[node->code.size()-1], "NOT_COMP");
            node->code.push_back(create_3AC("goto", "", "", node->label_false->name));
            create_3AC_goto(node->code[node->code.size()-1]);
        }

        
    }
    else if(node->name == "TYPEDARGS_LIST"){
        //DO nothing
    }else if(node->name == "CLASSDEF"){
        node->code.push_back(create_3AC_label(create_label_name(node->symbol_t->name)));
        node->code.push_back(create_3AC("beginclass", "", "", ""));
        create_3AC_code_placeholders(node->code[node->code.size()-1]);
        for(int i=0;i<node->children.size();i++){
            append_3AC(node, node->children[i]);
        }
        node->code.push_back(create_3AC("endclass", "", "", ""));
        create_3AC_code_placeholders(node->code[node->code.size()-1]);
    }
    else if(node->name == "FUNCDEF"){
        int size = calculate_class_type_size(node->symbol_t);
        node->code.push_back(create_3AC_label(create_label_name(node->symbol_t->parent->name + "." + node->symbol_t->name)));
        node->code.push_back(create_3AC("beginfunc", "", "", ""));
        create_3AC_code_placeholders(node->code[node->code.size()-1]);
        //pop the arguements

        if(node->children[2]->name == "TYPEDARGS_LIST"){
            for(int i=0;i<node->children[2]->children.size();i++){
                append_3AC(node, node->children[2]->children[i]);
            }
        }
        //here we manipulate the stack to allocate the local variables
        
        // node->code.push_back(create_3AC("-", "stack_pointer", to_string(size), "stack_pointer"));
        // create_3AC_code_expression(node->code[node->code.size()-1]);

        //check if it is init function
        

        // if(node->symbol_t->name == "__init__"){
        //    //allocate memory on the heap
        //     node->code.push_back(create_3AC("param", "", "", to_string(size)));
        //     create_3AC_pop_push(node->code[node->code.size()-1]);
        //     node->code.push_back(create_3AC("call", "memalloc", "1", ""));
        //     create_3AC_call(node->code[node->code.size()-1]);
        //     node->code.push_back(create_3AC("ret", "", "", node->symbol_t->parent->name + "." + node->symbol_t->name));
        //     create_3AC_pop_push(node->code[node->code.size()-1]);
        // }

        append_3AC(node, node->children[node->children.size()-1]);//last is always suite

        
        node->code.push_back(create_3AC("endfunc", "","", ""));
        create_3AC_code_placeholders(node->code[node->code.size()-1]);

    }else if(node->name == "RETURN_STMT"){
        //first append the return value/values
        if(node->children.size() == 2){
            append_3AC(node, node->children[1]);
            if(node->children[1]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[1]->addr = temp1;
            }
            //push the param
            node->code.push_back(create_3AC("leave", "", "", ""));
            create_3AC_code_placeholders(node->code[node->code.size()-1]);
            node->code.push_back(create_3AC("RETURN", "", "", node->children[1]->addr));
            create_3AC_pop_push(node->code[node->code.size()-1]);   
            node->code.push_back(create_3AC("return", "", "", ""));
            create_3AC_code_placeholders(node->code[node->code.size()-1]);
        }
        else{
            node->code.push_back(create_3AC("leave", "", "", ""));
            create_3AC_code_placeholders(node->code[node->code.size()-1]);
            node->code.push_back(create_3AC("return", "", "", ""));
            create_3AC_code_placeholders(node->code[node->code.size()-1]);
        }
    }
    //TODO: Translating function calls
    else if(node->name == "FUNC_CALL"){

        create_new_temp(node);

        table_entry* func_entry = node->func_entry;
        if(func_entry == NULL){
            cerr<<"Function "<<node->children[0]->lexval<<" not defined"<<endl;
            exit(1);
        } 
        if(func_entry->name == "range"){
            //skip this
            for(int i=0;i<node->children[1]->children.size();i++){
                append_3AC(node, node->children[1]->children[i]);
            }
            return;
        }
        if(func_entry->name == "len"){
            // cerr<<"Inside the len function"<<endl;


            //check if the argument is a string

            //get the symbol table entry of the node->children[1]->children[0]
            table_entry* entry = node->children[1]->children[0]->entry;
            // cerr<<entry->type<<endl;
            // cerr<<node->children[1]->children[0]->lexval<<endl;
            create_new_temp(node);
            if(entry->type == "LIST"){
                // node->addr = to_string(entry->list_size);
                node->code = vector<threeac*>();
                node->code.push_back(create_3AC("=", to_string(array_num_elements[entry->name]), "", node->addr));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type ="ASSIGN";
            }

            return ;
        }
       
        if(func_entry->type == "VOID"){
            //push the arguements
            // cerr<<func_entry->name<<"return type is void"<<endl;
            int args = 0;
            int size = 0;

            if(node->children.size()>1){
                //node->children[1] is arglist
                args = node->children[1]->children.size();
                // cerr<<"Args: "<<args<<endl;
                for(int i=node->children[1]->children.size()-1;i>=0;i--){
                    append_3AC(node, node->children[1]->children[i]);
                }
                
                for(int i=node->children[1]->children.size()-1;i>=0;i--){
                    size+=type_size[node->children[1]->children[i]->type];   
                    if(node->children[1]->children[i]->addr.substr(0,1) == "*"){
                        //perform the load operation
                        string temp1 = "t" + to_string(temp_count);
                        temp_count++;
                        node->code.push_back(create_3AC("=", node->children[1]->children[i]->addr, "", temp1));
                        create_3AC_code_equal(node->code[node->code.size()-1]);
                        node->code[node->code.size()-1]->equal_type = "LOAD";
                        if(find_temp(node->children[1]->children[i]->addr)){
                            string_temps.push_back(temp1);
                        }
                        node->children[1]->children[i]->addr = temp1;
                    }
                    if(find_temp(node->children[1]->children[i]->addr)){
                        //we need to make rdi 1
                        node->code.push_back(create_3AC("print_string", "", "", ""));
                        print_string++;
                        create_3AC_code_placeholders(node->code[node->code.size()-1]);

                        
                    }
                    node->code.push_back(create_3AC("param", "", "", node->children[1]->children[i]->addr));
                    create_3AC_pop_push(node->code[node->code.size()-1]);
                }
            }

            if(node->is_func_dot){
                // the object is node->func_dot_obj or it may be a class
                if(get_obj_class(node->func_dot_obj) == NULL){
                    node->code.push_back(create_3AC("param", "", "", node->func_dot_obj));
                    create_3AC_pop_push(node->code[node->code.size()-1]);
                }
            }
            if(size!=0){
                // node->code.push_back(create_3AC("-", "stack_pointer",to_string(size),"stack_pointer"));
                // create_3AC_code_expression(node->code[node->code.size()-1]);
            }
            node->code.push_back(create_3AC("call", func_entry->symbol_table->parent->name + "." +func_entry->name , to_string(args) , ""));
            create_3AC_call(node->code[node->code.size()-1]);
            if(size!=0){
                // node->code.push_back(create_3AC("+", "stack_pointer",to_string(size),"stack_pointer"));
                // create_3AC_code_expression(node->code[node->code.size()-1]);
            }

            if(func_entry->name == "__init__"){
                // node->code.push
            }else{

                node->code.push_back(create_3AC("ret", "", "", ""));
                create_3AC_pop_push(node->code[node->code.size()-1]);
            }
            
        }else{
            // cerr<<func_entry->name<<"return type is not void"<<endl;
            create_new_temp(node);
            //push the arguements
            int args = 0;
            int size = 0;

            //check if we are calling a constructor function
            //check if it is a class
            if(func_entry->name == "__init__"){
                //get the parent(class symbol table)
                symbol_table* class_table = func_entry->symbol_table->parent;
                //get the size of the class
                size = calculate_class_obj_type_size(class_table);
                //allocate the memory
                node->code.push_back(create_3AC("param", "", "", to_string(size)));
                create_3AC_pop_push(node->code[node->code.size()-1]);
                node->code.push_back(create_3AC("call", "memalloc", "1", ""));
                create_3AC_call(node->code[node->code.size()-1]);
                node->code.push_back(create_3AC("ret", "", "", node->addr));
                create_3AC_pop_push(node->code[node->code.size()-1]);

            }

            if(node->children.size()>1){
                //node->children[1] is arglist
                args = node->children[1]->children.size();
                for(int i=node->children[1]->children.size()-1;i>=0;i--){
                    append_3AC(node, node->children[1]->children[i]);
                }
                for(int i=node->children[1]->children.size()-1;i>=0;i--){
                    size+=type_size[node->children[1]->children[i]->type];
                    if(node->children[1]->children[i]->addr.substr(0,1) == "*"){
                        //perform the load operation
                        string temp1 = "t" + to_string(temp_count);
                        temp_count++;
                        node->code.push_back(create_3AC("=", node->children[1]->children[i]->addr, "", temp1));
                        create_3AC_code_equal(node->code[node->code.size()-1]);
                        node->code[node->code.size()-1]->equal_type = "LOAD";
                        if(find_temp(node->children[1]->children[i]->addr)){
                            string_temps.push_back(temp1);
                        }
                        node->children[1]->children[i]->addr = temp1;
                        
                        // string_temps.push_back(node->children[1]->children[i]->addr);
                    }
                    node->code.push_back(create_3AC("param", "", "", node->children[1]->children[i]->addr));
                    create_3AC_pop_push(node->code[node->code.size()-1]);
                }
            }
            if(size!=0){
                // node->code.push_back(create_3AC("-", "stack_pointer",to_string(size),"stack_pointer"));
                // create_3AC_code_expression(node->code[node->code.size()-1]);
            }
            

            if(func_entry->name == "__init__"){
                // node->code.push_back(create_3AC("call", func_entry->symbol_table->parent->name+ "."+func_entry->name, to_string(args), ""));
                //push the allocated memory as an argue 
                //if(node->is_func_dot){
                //    cerr<<"====================="<<endl;
                //    cerr<<"Inside the func_dot"<<node->func_dot_obj<<endl;
                //}else{
                    node->code.push_back(create_3AC("param", "", "", node->addr));
                    create_3AC_pop_push(node->code[node->code.size()-1]);
                //}
            }
            else if(node->is_func_dot){
                // the object is node->func_dot_obj or it may be a class
                if(get_obj_class(node->func_dot_obj) == NULL){
                    //get the offset of the variable
                    //check if it is a object
                    //push the node->func_dot_obj as an argument
                    node->code.push_back(create_3AC("param", "", "", node->func_dot_obj));
                    create_3AC_pop_push(node->code[node->code.size()-1]);
                }
            }
            node->code.push_back(create_3AC("call", func_entry->symbol_table->parent->name+ "."+func_entry->name, to_string(args), ""));
            create_3AC_call(node->code[node->code.size()-1]);
            if(size!=0){
                // node->code.push_back(create_3AC("+", "stack_pointer",to_string(size),"stack_pointer"));
                // create_3AC_code_expression(node->code[node->code.size()-1]);
            }
            if(func_entry->name == "__init__"){
                node->code.push_back(create_3AC("ret", "", "", ""));
                create_3AC_pop_push(node->code[node->code.size()-1]);
            }else{
                node->code.push_back(create_3AC("ret", "", "", node->addr));
                create_3AC_pop_push(node->code[node->code.size()-1]);
            }
            
        }
        
    }else if(node->name == "ARGLIST"){
        //do nothing, generate the for its children recursively
    }
    //TODO: If else stmts
    else if(node->name == "IF_STMT"){
        //append the condition
        append_3AC(node, node->children[0]);
        //push back the label for the true condition
        node->code.push_back(create_3AC_label(node->children[0]->label_true));
        //append the suite
        append_3AC(node, node->children[1]);

        //if stmt done, append the next label
        node->code.push_back(create_3AC("goto", "", "", node->label_next->name));
        create_3AC_goto(node->code[node->code.size()-1]);

        if(node->children.size() == 3 && node->children[2]->name == "ELSE_BLOCK"){
            //implies there is an else block
            
            node->code.push_back(create_3AC_label(node->children[0]->label_false));
            append_3AC(node, node->children[2]);

        }

        if(node->children.size() == 3 && node->children[2]->name == "ELIF_BLOCK"){
            //implies there is an elif block only
            node->code.push_back(create_3AC_label(node->children[0]->label_false));
            append_3AC(node, node->children[2]);
        }
        
        if(node->children.size()==4){
            //implies there is an elif block and an else block
            node->code.push_back(create_3AC_label(node->children[0]->label_false));
            append_3AC(node, node->children[2]);
            append_3AC(node, node->children[3]);
        }
        
        node->code.push_back(create_3AC_label(node->label_next));
    }else if(node->name == "THEN"){
        append_3AC(node, node->children[0]);
    } else if(node->name == "ELSE_BLOCK"){
        append_3AC(node, node->children[0]);
    }else if(node->name == "ELIF_BLOCK"){
        if(node->children.size() == 3){
            append_3AC(node, node->children[2]);
        }
        append_3AC(node, node->children[0]);
        node->code.push_back(create_3AC_label(node->children[0]->label_true));
        append_3AC(node, node->children[1]);
        node->code.push_back(create_3AC("goto", "", "", node->label_next->name));
        create_3AC_goto(node->code[node->code.size()-1]);
        node->code.push_back(create_3AC_label(node->children[0]->label_false));

    }
    else if(node->name == "COMPARISON"){
        
        // cerr<<"Inside comparison"<<endl;

        if(node->compare_type_str){
            // cerr<<"STRING COMPARISON"<<endl;
        }
        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
        append_3AC(node, node->children[2]);
        if(node->children[2]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[2]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[2]->addr = temp1;
        }
       
        if(node->label_true && node->label_false){
            node->code.push_back(create_3AC("if", node->children[0]->addr + " "+ node->children[1]->name + " "+ node->children[2]->addr , "goto" , node->label_true->name));
            node->code[node->code.size()-1]->is_comp_str = node->compare_type_str;
            node->code[node->code.size()-1]->comp1 = node->children[0]->addr;
            node->code[node->code.size()-1]->comp2 = node->children[2]->addr;
            create_3AC_if(node->code[node->code.size()-1], node->children[1]->name);
            node->code.push_back(create_3AC("goto", "", "", node->label_false->name));
            create_3AC_goto(node->code[node->code.size()-1]);
        }else{//here it is allocated
            create_new_temp(node); //this holds the value of the final comparison
            node->code.push_back(create_3AC(node->children[1]->name, node->children[0]->addr, node->children[2]->addr, node->addr));
            create_3AC_code_expression(node->code[node->code.size()-1]);
            // node->code[node->code.size()-1]->equal_type = "ASSIGN";
        }

    }else if(node->name == "OR_TEST"){
        if(node->label_false){
            append_3AC(node, node->children[0]);
            if(node->children[0]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[0]->addr = temp1;
            }
            node->code.push_back(create_3AC_label(node->children[0]->label_false));
            append_3AC(node, node->children[2]);
            if(node->children[2]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[2]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[2]->addr = temp1;
            }
        }else {
            //op OR
            create_new_temp(node);
            append_3AC(node, node->children[0]);
            if(node->children[0]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[0]->addr = temp1;
            }
            node->code.push_back(create_3AC("OR", node->children[0]->addr, node->children[2]->addr, node->addr));
            create_3AC_code_expression(node->code[node->code.size()-1]);

        }
    }else if(node->name == "AND_TEST"){
        // cerr<<"AND TEST"<<endl;
        if(node->label_true){
            append_3AC(node, node->children[0]);
            if(node->children[0]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[0]->addr = temp1;
            }
            node->code.push_back(create_3AC_label(node->children[0]->label_true));
            append_3AC(node, node->children[2]);
            if(node->children[2]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[2]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[2]->addr = temp1;
            }
        }else{
            //op AND
            create_new_temp(node);
            append_3AC(node, node->children[0]);
            if(node->children[0]->addr.substr(0,1) == "*"){
                //perform the load operation
                string temp1 = "t" + to_string(temp_count);
                temp_count++;
                node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
                create_3AC_code_equal(node->code[node->code.size()-1]);
                node->code[node->code.size()-1]->equal_type = "LOAD";
                node->children[0]->addr = temp1;
            }
            node->code.push_back(create_3AC("AND", node->children[0]->addr, node->children[2]->addr, node->addr));
            create_3AC_code_expression(node->code[node->code.size()-1]);
        }
       
    }
    // else if(node->name == "NOT_TEST"){
        // if(node->label_true){
        //     append_3AC(node, node->children[1]);
        //     if(node->children[1]->addr.substr(0,1) == "*"){
        //         //perform the load operation
        //         string temp1 = "t" + to_string(temp_count);
        //         temp_count++;
        //         node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
        //         create_3AC_code_equal(node->code[node->code.size()-1]);
        //         node->code[node->code.size()-1]->equal_type = "LOAD";
        //         node->children[1]->addr = temp1;
        //     }
        // }else{
        //     create_new_temp(node);
        //     append_3AC(node, node->children[1]);
        //     if(node->children[1]->addr.substr(0,1) == "*"){
        //         //perform the load operation
        //         string temp1 = "t" + to_string(temp_count);
        //         temp_count++;
        //         node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
        //         create_3AC_code_equal(node->code[node->code.size()-1]);
        //         node->code[node->code.size()-1]->equal_type = "LOAD";
        //         node->children[1]->addr = temp1;
        //     }
        //     node->code.push_back(create_3AC("NOT", node->children[1]->addr, "", node->addr));
        //     create_3AC_code_expression(node->code[node->code.size()-1]);
        // }

    //     append_3AC(node, node->children[1]);
    // }
    else if(node->name == "BOOLVAL"){
        if(node->lexval == "True"){
            if(node->label_true == NULL){
                node->addr = "1";
                
            }else{
                node->code.push_back(create_3AC("goto", "", "", node->label_true->name));
                create_3AC_goto(node->code[node->code.size()-1]);
            }
        }else{
            if(node->label_false == NULL){
                node->addr = "0";

            }else{
                node->code.push_back(create_3AC("goto", "", "", node->label_false->name));
                create_3AC_goto(node->code[node->code.size()-1]);
            }
        }

    }else if(node->name == "SUITE"){
        for(int i=0;i<node->children.size();i++){
            append_3AC(node, node->children[i]);
        }
    }
    else if(node->name == "WHILE_STMT"){
        // cerr<<"While stmt"<<endl;
        node->code.push_back(create_3AC_label(node->label_begin));
        //condition check
        append_3AC(node, node->children[0]);
        if(node->children[0]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[0]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[0]->addr = temp1;
        }
        node->code.push_back(create_3AC_label(node->children[0]->label_true));
        //append suite
        append_3AC(node, node->children[1]);
        // cerr<<"While stmt"<<endl;

        node->code.push_back(create_3AC("goto", "", "", node->label_begin->name));
        create_3AC_goto(node->code[node->code.size()-1]);
        node->code.push_back(create_3AC_label(node->label_next));
        // cerr<<"While stmt done"<<endl;
    }else if(node->name == "FOR_STMT"){
        append_3AC(node, node->children[3]);

       
        create_new_temp(node->children[3]);
        //create a new temp for the iterator
        if(node->children[1]->addr.substr(0,1) == "*"){
            //perform the load operation
            string temp1 = "t" + to_string(temp_count);
            temp_count++;
            node->code.push_back(create_3AC("=", node->children[1]->addr, "", temp1));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "LOAD";
            node->children[1]->addr = temp1;
        }

        if(node->children[3]->children[1]->children.size()==2){
            //both provided
            node->code.push_back(create_3AC("=", node->children[3]->children[1]->children[0]->addr, "", node->children[3]->addr));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "ASSIGN";
        }else{
            //only one provided
            node->code.push_back(create_3AC("=", "0", "", node->children[3]->addr));
            create_3AC_code_equal(node->code[node->code.size()-1]);
            node->code[node->code.size()-1]->equal_type = "ASSIGN";
        }


        node->code.push_back(create_3AC_label(node->label_begin));

        node->code.push_back(create_3AC("=", node->children[3]->addr, "", node->children[1]->addr));
        create_3AC_code_equal(node->code[node->code.size()-1]);
        node->code[node->code.size()-1]->equal_type = "ASSIGN";
        
      


        //end the comparison check
        if(node->children[3]->children[1]->children.size()==2){
            node->code.push_back(create_3AC("if", node->children[3]->addr + " LT " + node->children[3]->children[1]->children[1]->addr, "goto", node->children[3]->label_true->name));
            node->code[node->code.size()-1]->comp1 = node->children[3]->addr;
            node->code[node->code.size()-1]->comp2 = node->children[3]->children[1]->children[1]->addr;
            create_3AC_if(node->code[node->code.size()-1], "LT");
        }else{
            node->code.push_back(create_3AC("if", node->children[3]->addr + " LT "+ node->children[3]->children[1]->children[0]->addr ,"goto", node->children[3]->label_true->name));
            node->code[node->code.size()-1]->comp1 = node->children[3]->addr;
            node->code[node->code.size()-1]->comp2 = node->children[3]->children[1]->children[0]->addr;
            create_3AC_if(node->code[node->code.size()-1], "LT");
        }
        node->code.push_back(create_3AC("goto", "", "", node->label_next->name)); //comparison false goes to end
        create_3AC_goto(node->code[node->code.size()-1]);
        node->code.push_back(create_3AC_label(node->children[3]->label_true));
        //children[4] has the suite
        append_3AC(node, node->children[4]);
        //increment the iterator
        //push back the label iter
        node->code.push_back(create_3AC_label(node->label_iter));
        node->code.push_back(create_3AC("+", node->children[3]->addr, "1", node->children[3]->addr));
        create_3AC_code_expression(node->code[node->code.size()-1]);
        //allocate the iterator to the actual node->children[1]->addr

        node->code.push_back(create_3AC("goto", "", "", node->label_begin->name));
        create_3AC_goto(node->code[node->code.size()-1]);
        node->code.push_back(create_3AC_label(node->label_next));
    }//TODO: break and cont
    else if(node->name == "BREAK"){
        node->code.push_back(create_3AC("goto", "", "", node->label_next->name));
        create_3AC_goto(node->code[node->code.size()-1]);
    }else if(node->name == "CONTINUE"){
        // Incrementing the iterator is important for the case of while loop


        node->code.push_back(create_3AC("goto", "", "", node->label_next->name));
        create_3AC_goto(node->code[node->code.size()-1]);
    }else{
        for(int i=0;i<node->children.size();i++){
            append_3AC(node, node->children[i]);
        }

    }
    
    //TODO : class definitions
    //I dont want to support multiple and, or and not statements

}


void print_3AC(node_ast* node, ofstream &file){
    static int count = 0;
    // if (remove("3ac_output.txt") != 0) {
    //     cout << "Error: Unable to delete file 3ac_output.txt" << endl;
    //     return;
    // }

    // ofstream file("3ac_output.txt", ios::out);
    for(int i=0;i<node->children.size();i++){
        print_3AC(node->children[i], file);
    }

    for(int i=0;i<node->code.size();i++){
        file << count << ":\t" << node->code[i]->triplet_code;
        ++count;
    }

    // file.close();
}

void print_3AC_wrapper(node_ast* node){
    string input_file_str = input_file;
    std::size_t found = input_file_str.find_last_of(".");
    std::string wihtout_extension = input_file_str.substr(0, found);
    string tac_name = wihtout_extension + ".tac";
    ofstream file(tac_name, ios::out);
    print_3AC(node, file);
    file.close();
}

void handle_label_false_elif_block(node_ast* node){
    // simplify_tree(node);
    //case only elifs
    if(node->children.size()==3 && node->children[2]->name == "ELIF_BLOCK"){
        //first handle the if
        node->children[0]->label_false = create_new_label();
        node->children[1]->label_next = node->label_next;
        node->children[2]->label_next = node->label_next;
        node = node->children[2];


        while(node->children.size()==3 && node->children[2]->name == "ELIF_BLOCK"){
            node->children[0]->label_false = create_new_label();
            node->children[1]->label_next = node->label_next;
            node->children[2]->label_next = node->label_next;
            node = node->children[2];
        }

        //last elif
        node->children[0]->label_false = create_new_label();
        node->children[1]->label_next = node->label_next;

    }

    //case elifs and else
    else if(node->children.size() == 4 && node->children[3]->name == "ELSE_BLOCK"){
        // node->children[0]->label_false = node->children[2]->children[0]->label_true;//already set 

        node->children[0]->label_false = create_new_label();
        node->children[1]->label_next = node->label_next;
        node->children[2]->label_next = node->label_next;

        node = node->children[2];
        
        while(node->children.size()==3 && node->children[2]->name == "ELIF_BLOCK"){
            node->children[0]->label_false = create_new_label();
            node->children[1]->label_next = node->label_next;
            node->children[2]->label_next = node->label_next;
            node = node->children[2];
        }

        //last elif,
        node->children[0]->label_false = create_new_label();
        node->children[1]->label_next = node->label_next;
        node->children[2]->label_next = node->label_next;

    }

    //case only if && else

    else if(node->children.size()==3 && node->children[2]->name == "ELSE_BLOCK"){
        //append the false label of the last elif to the next label
        node->children[0]->label_false = create_new_label();//if block part
        node->children[1]->label_next = node->label_next;//for completing of the if block
        node->children[2]->label_next = node->label_next;//for the else block

    }
    else{
        node->children[0]->label_false = node->label_next;
        node->children[1]->label_next = node->label_next;

    }

}


void handle_inherited_attributes(node_ast* node){

    if(node->name == "OR_TEST"){

        node->children[0]->label_true = node->label_true;
        node->children[0]->label_false = create_new_label();
        node->children[2]->label_true = node->label_true;
        node->children[2]->label_false = node->label_false;
    }

    if(node->name == "AND_TEST"){
        node->children[0]->label_true = create_new_label();
        node->children[0]->label_false = node->label_false;
        node->children[2]->label_true = node->label_true;
        node->children[2]->label_false = node->label_false;
    }

    if(node->name == "NOT_TEST"){
        node->children[1]->label_true = node->label_false;
        node->children[1]->label_false = node->label_true;
    }
}




struct node_ast* create_node(string name){
    // struct node_ast* node = (struct node_ast*)malloc(sizeof(struct node_ast));
    struct node_ast* node = new struct node_ast;
    node->name = name;
    node->lexval = "";
    return node;
}

struct node_ast* create_node_lex(string name, string lexval){
    // struct node_ast* node = (struct node_ast*)malloc(sizeof(struct node_ast));
    struct node_ast* node = new struct node_ast;
    node->name = name;
    node->lexval = lexval;
    return node;
}


void simplify_tree(node_ast* node){
    
    if(node==NULL){
        return;
    }
    for(int i = 0; i < node->children.size(); i++){
        simplify_tree(node->children[i]);
    }
    //traverse the children and if only one is not an "EMPTY_PROD" then keep it
    vector<node_ast*> new_children;
    for(int i = 0; i < node->children.size(); i++){
        if(node->children[i]->name != "EMPTY_PROD"){
            new_children.push_back(node->children[i]);
        }
    }
    node->children = new_children;

    if(node->children.size() == 1 && node->name!="NAME" && node->name!="THEN" && node->name!="ARGLIST" && node->name != "ELSE_BLOCK" && node->name != "ELIF_BLOCK" && node->name!="FUNC_CALL" && node->name!="TYPEDARGS_LIST" && node->name!="RETURN_STMT" && node->name!="SUITE" && node->name!="TESTLIST_COMP" && node->name!="TFPDEF"){
        
        if(node->children[0]->name == "SUITE" ){
            
        }else{

            // copy all the fields
            node->func_entry = node->children[0]->func_entry;
            node->name = node->children[0]->name;
            node->lexval = node->children[0]->lexval;
            if(node->children[0]->type_set){
                node->type = node->children[0]->type;
                node->type_set = true;
            }
            node->label_begin = node->children[0]->label_begin;
            node->array_entry = node->children[0]->array_entry; 
            node->label_true = node->children[0]->label_true;
            node->label_false = node->children[0]->label_false;
            node->label_next = node->children[0]->label_next;
            node->label_iter = node->children[0]->label_iter;
            node->symbol_t = node->children[0]->symbol_t;
            node->arr_arg = node->children[0]->arr_arg;
            node->is_dot = node->children[0]->is_dot;
            node->dot_name = node->children[0]->dot_name;
            node->dot_obj = node->children[0]->dot_obj;
            node->obj_class_name = node->children[0]->obj_class_name;
            node->is_func_dot = node->children[0]->is_func_dot;
            node->func_dot_name = node->children[0]->func_dot_name;
            node->func_dot_obj = node->children[0]->func_dot_obj;
            node->compare_type_str = node->children[0]->compare_type_str;
            node->children = node->children[0]->children;
            
            
        }
        
    }


}

void print_tree(struct node_ast* root_node){
    cout << root_node->name << endl;
    for(int i = 0; i < root_node->children.size(); i++){
        print_tree(root_node->children[i]);
    }

}

std::string replaceQuotes(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (c == '"') {
            result += "\\\"";
        } else {
            result += c;
        }
    }
    return result;
}
void printDotHelper(node_ast* node, int& nodeId, int parentId) {
    if (!node) return; // Safety check

    int currentNodeId = nodeId++; // Assign an ID to the current node
    // Print the node itself. You might want to include additional formatting here.
    string lexval;
    if(node->name == "STRING"){
        lexval = replaceQuotes(node->lexval);
    }else{
        lexval = node->lexval;
    }
    if(node->lexval != "" && node->lexval != " "){

        cout << "  " << currentNodeId << " [label=\"" << node->name << " : " << lexval << "\"];" << endl;
    }else{
        cout << "  " << currentNodeId << " [label=\"" << node->name << "\"];" << endl;
    }

    // If this is not the root node, print the edge from its parent
    if (parentId != -1) {
        cout << "  " << parentId << " -> " << currentNodeId << ";" << endl;
    }

    // Recur for each child
    for (size_t i = 0; i < node->children.size(); ++i) {
        printDotHelper(node->children[i], nodeId, currentNodeId);
    }
}
void printDot(node_ast* root) {
    std::ofstream outFile(dotfile_path);
    std::streambuf* coutBuf = std::cout.rdbuf();
    std::cout.rdbuf(outFile.rdbuf());
    
    int initialNodeId = 0;
    // remove_empty_productions(root);
    simplify_tree(root);
    std::cout << "digraph G {" << std::endl;
    printDotHelper(root, initialNodeId, -1);
    std::cout << "}" << std::endl;
    std::cout.rdbuf(coutBuf);
    outFile.close();

    if (strcmp(ast_type.c_str(), "pdf") == 0){
        string input_file_str = input_file;
        std::size_t last_dot_position = input_file_str.find_last_of(".");
        std::string without_extension = input_file_str.substr(0, last_dot_position);
        string dotfile_path_str = dotfile_path;
        std::string cmd_graph = "dot -Tpdf -o " + without_extension + ".pdf " + dotfile_path_str;
        const char* cmd = cmd_graph.c_str();
        system(cmd);
    } else if (ast_type == "png"){
        string input_file_str(input_file);
        std::size_t last_dot_position = input_file_str.find_last_of(".");
        std::string without_extension = input_file_str.substr(0, last_dot_position);
        string dotfile_path_str(dotfile_path);
        std::string cmd_graph = "dot -Tpng -o " + without_extension + ".png " + dotfile_path_str;
        const char* cmd = cmd_graph.c_str();
        system(cmd);
    }else {
        cout << "Invalid argument used for --graph. Not generating the graph file" << endl;
    }
}


void error_report(string msg, int lineno){
    cerr << "Line " << lineno << ": " << msg << endl;
    exit(1);
}

struct node_ast* parse_tree_to_ast_expr_right_ass(vector<node_ast*> expr){

    //every second node is an operator, I need to create a left associative ast

    //base case
    
    if(expr.size() == 1){
        return expr[0];
    }
    struct node_ast* new_node = create_node_lex(expr[1]->name, expr[1]->lexval);

    new_node->children.push_back(expr[0]);
    new_node->children.push_back(parse_tree_to_ast_expr_right_ass(vector<node_ast*>(expr.begin()+2, expr.end())));
    return new_node;


}

struct node_ast* parse_tree_to_ast_expr_left_ass(vector<node_ast*> expr){
    
    
    if(expr.size() == 1){
        return expr[0];
    }

    //create a new node
    struct node_ast* new_node = create_node_lex(expr[expr.size()-2]->name,expr[expr.size()-2]->lexval);
    new_node->children.push_back(parse_tree_to_ast_expr_left_ass(vector<node_ast*>(expr.begin(), expr.end()-2)));
    new_node->children.push_back(expr[expr.size()-1]);
    return new_node;
}
