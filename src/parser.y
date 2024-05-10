%define parse.error verbose
%{
    #include <stdio.h>
    #include <iostream>
    #include <fstream>
    #include <string>
    #include <cstdlib>
    #include <stack>
    #include <unordered_map>
    #include "src/x86.cpp"


    using namespace std;
    extern int yylex();
    extern int yylineno;
    extern int getLineNo(int line, string text);
    extern symbol_table* global_table;

    extern int error_line;
    extern int verbose;
    extern string ast_type;
    extern string dotfile_path;
    extern char* input_file;
    void yyerror(const char *s) {
        fprintf(stderr, "Syntax Error detected by bison at : line %d : %s\n", yylineno, s);
        exit(1);
    }
    extern FILE *yyin;

    extern stack<int> paren;
    extern stack<int> brace;
    extern stack<int> bracket;
    extern stack<symbol_table*> table_stack;

    struct node_ast* CLASS_NODE_TEMP;
    struct node_ast* FUNC_NODE_TEMP;
    stack<node_ast*> LOCAL_TEMP;

    extern int offset; //from the symbol_table.cpp file

    extern unordered_map<string,int> type_size;
%}

%union{
    int int_type;
    double float_type;
    struct node_ast* node_ptr;
    char* str;
}

%left PLUS MINUS
%left STAR SLASH
%nonassoc UMINUS

%token TOKEN_INDENT
%token TOKEN_DEDENT
%token TOKEN_EOF
%token TOKEN_ERROR
%token ENDMARKER
%token TRIPLE_DOT

%token INT
%token FLOAT
%token STR
%token BOOL
%token COMPLEX
%token PRINT
%token RANGE

// %token KEYWORD
%token FALSE
%token NONE
%token TRUE
%token AS
%token ASSERT
%token ASYNC
%token AWAIT
%token BREAK
%token CLASS
%token CONTINUE
%token DEF
%token DEL
%token ELIF
%token ELSE
%token EXCEPT
%token FINALLY
%token FOR
%token FROM
%token GLOBAL
%token IF
%token IMPORT
%token IN
%token IS
%token LAMBDA
%token NONLOCAL
%token PASS
%token RAISE
%token RETURN
%token TRY
%token WHILE
%token WITH
%token YIELD
%token LIST
%token LEN

%token<str> NUMBER
%token<str> STRING

%token LPAREN
%token RPAREN
%token LBRACKET
%token RBRACKET
%token LBRACE
%token RBRACE
%token COMMA
%token COLON
%token DOT
%token SEMICOLON
%token AT
%token ARROW
%token FLOORDIV_ASSIGN

%token PLUS
%token MINUS
%token STAR
%token SLASH
%token DOUBLESLASH
%token PERCENT
%token DOUBLESTAR

%token EQ
%token NE
%token GT
%token LT
%token GE
%token LE

%token AND
%token OR
%token NOT

%token<str> NAME

%token AMPER
%token PIPE
%token CARET
%token TILDE
%token LSHIFT
%token RSHIFT

%token EQUAL
%token PLUSEQUAL
%token MINUSEQUAL
%token STAREQUAL
%token SLASHEQUAL
%token PERCENTEQUAL
%token DOUBLESTAREQUAL
%token AMPEREQUAL
%token PIPEEQUAL
%token CARETEQUAL
%token LSHIFTEQUAL
%token RSHIFTEQUAL

%token NEWLINE

%type<node_ptr> file_input file_lines stmt extra_keywords else_block global_stmt typedargslist trailer_star if_stmt_line if_stmt compound_stmt test atom argument arglist_line arglist classdef suite expr testlist_line testlist  sliceop subscript subscriptlist_line subscriptlist trailer testlist_comp_line testlist_comp  atom_expr power factor term_line term arith_expr_line arith_expr shift_expr_line shift_expr and_expr_line and_expr xor_expr_line xor_expr expr_line comp_op comparison not_test and_test_line and_test or_test_line or_test stmts simple_stmt for_stmt  while_stmt funcdef assert_stmt return_stmt continue_stmt break_stmt flow_stmt pass_stmt del_stmt augassign expr_stmt simple_stmt_line small_stmt simple_stmt_lines tfpdef typedargslist_starred_line typedargslist_starred parameters funcdef_line
%%
file_input
    : file_lines ENDMARKER {
        $$ = create_node("FILE_INPUT");
        struct node_ast* node_2 = create_node("ENDMARKER");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        // add_entries_to_symbol_table(global_table, $1->entries);
        free($1);
        $$->children.push_back(node_2);
        printDot($$);
        
        // print_global_table();
        //TODO : generate 3AC using the tree
        add_data_section($$);
        generate_3AC($$);
        
        // cerr<<"GENERATING 3AC"<<endl;
        // //TODO: print the 3AC on the  file
        print_3AC_wrapper($$);

        //TODO : print the symbol table
        print_csv();
        generate_text_section($$);
        exit(0);
    }
    ;

file_lines
    : {
        $$ = create_node("EMPTY_PROD");
        $$->entries = vector<table_entry*>();
    }
    | file_lines stmt {
        $$ = create_node("FILE_LINES");

        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        $$->children.push_back($2);
        $$->entries = $1->entries;
        for (int i = 0; i < $2->entries.size(); i++){
            // cout<<"adding entry to file_lines"<<$2->entries[i]->name<<endl;
            add_entry_to_entries($$->entries, $2->entries[i]);
        }
        
        free($1);
    }
    | file_lines NEWLINE {
        $$ = create_node("FILE_LINES");
        $$->entries = $1->entries;
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        free($1);
    }
    ;

funcdef
    : DEF NAME{
        $<node_ptr>$ = create_node("FUNCDEF");
        struct node_ast* node_1 = create_node("DEF");
        $<node_ptr>$->children.push_back(node_1);
        struct node_ast* node_2 = create_node_lex("NAME", $2);
        $<node_ptr>$->children.push_back(node_2);
        $<node_ptr>$->symbol_t = create_symbol_table_function($2, vector<table_entry*>(), "AWAITED", "function", yylineno);
        FUNC_NODE_TEMP = $<node_ptr>$;
        allocate_parent_and_child($<node_ptr>$->symbol_t);
        add_symbol_table_to_class_entries($<node_ptr>$->symbol_t);
        table_stack.push($<node_ptr>$->symbol_t);

    } parameters{
        // $<node_ptr>$ = create_node("FUNCDEF");
        // struct node_ast* node_1 = create_node("DEF");
        // $<node_ptr>$->children.push_back(node_1);
        // struct node_ast* node_2 = create_node_lex("NAME", $<str>2);
        // $<node_ptr>$->children.push_back(node_2);
        // $<node_ptr>$->symbol_t = create_symbol_table_function($2, vector<table_entry*>(), "AWAITED", "function", yylineno);
        // FUNC_NODE_TEMP = $<node_ptr>$;
        // allocate_parent_and_child($<node_ptr>$->symbol_t);
        $<node_ptr>$ = FUNC_NODE_TEMP;
        simplify_tree($4);
        $<node_ptr>$->children.push_back($4);  
        $<node_ptr>$->symbol_t->parameters = $4->entries;
        
       

    } funcdef_line{
        $$ = FUNC_NODE_TEMP;
        // $$->children.push_back($4);
        // $$->symbol_t->parameters = $4->entries;
        
        simplify_tree($6);
        //check if init is the function name
        // cerr<<"function name is"<<$2<<endl;
        
        if(strcmp($2, "__init__")==0){
            $$->symbol_t->return_type = $$->symbol_t->parent->name;
            // add_scope_entries_to_parent_init();
            // we need to add the entries of the function to the parent class, but only those 
            //starting with self.

            for(int i=0;i<table_stack.top()->entries.size();i++){
                //add all the self entries to parent
                if(table_stack.top()->entries[i]->name.substr(0,4) == "self"){
                    add_entry_to_entries($$->symbol_t->parent->entries, table_stack.top()->entries[i]);
                }
            }
            
            table_stack.top()->my_table_entry->type = $$->symbol_t->parent->name;
        }else{
            $$->symbol_t->return_type = $6->type; //set the return type

        }
       
        for(int i = 0; i < $6->children.size(); i++){
            $$->children.push_back($6->children[i]);
        }
        
        free($6);
    }
    ;

funcdef_line
    : {
        table_stack.top()->my_table_entry->type = "VOID";
    }COLON suite {
        $$ = create_node("FUNCDEF_LINE");
        $$->children.push_back($3);
        $$->type = "VOID";
        // add entries of suite to $$->entries
        $$->entries = $3->entries;

    }
    | error { error_line = yylineno; } suite {
        error_report("Syntax Error: ':' missing", yylineno);
    }
    | ARROW atom_expr {
        table_stack.top()->my_table_entry->type = $2->type;
    }COLON suite {
        $$ = create_node("FUNCDEF_LINE");
        $$->children.push_back($2);
        $$->children.push_back($5);
        $$->type = $2->type;
        $$->entries = $5->entries;
        

    }
    | ARROW test error { error_line = yylineno; } suite {
        error_report("Syntax Error: ':' missing", yylineno);
    }
    ;

parameters
    : LPAREN RPAREN {
        $$ = create_node("EMPTY_PROD");
        $$->entries = vector<table_entry*>();
    }
    | LPAREN error {

        error_report("Syntax Error: ')' expected for '(' at ", paren.top());
    }
    | LPAREN typedargslist RPAREN {
        $$ = create_node("PARAMETERS");
        $$->children.push_back($2);
        $$->entries = $2->entries;

    }
    | LPAREN typedargslist error{
        error_report("Syntax Error: ')' expected for '(' at ", paren.top());
    }
    ;

typedargslist
    : tfpdef typedargslist_starred {
        $$ = create_node("TYPEDARGS_LIST");
        $$->children.push_back($1);
        
        // $$->entries = $2->entries;
        $$->entries.push_back($1->entry);
        add_entry_to_symbol_table($1->entry);
        // add_entry_to_entries($$->entries, $1->entry);
        for(int i=0;i<$2->entries.size();i++){
            add_entry_to_entries($$->entries, $2->entries[i]);
        }
        
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        free($2);

    }
    ;


typedargslist_starred
    : {
        $$ = create_node("EMPTY_PROD");
        $$->entries = vector<table_entry*>();
    }
    | typedargslist_starred typedargslist_starred_line{
        $$ = create_node("TYPEDARGS_LIST_STARRED");
        
        $$->entries = $1->entries;
        add_entry_to_entries($$->entries, $2->entry);
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        free($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        free($2);
    }
    ;

typedargslist_starred_line
    : COMMA tfpdef  {
        $$ = create_node("TYPEDARGS_LIST_STARRED_LINE");
        $$->children.push_back($2);
        $$->entry = $2->entry;
        add_entry_to_symbol_table($2->entry);
    }
    | COMMA tfpdef EQUAL test{
        $$ = create_node("TYPEDARGS_LIST_STARRED_LINE");
        $$->children.push_back($2);
        $$->children.push_back(create_node("EQUAL"));
        $$->children.push_back($4);
        $$->entry = $2->entry;
        add_entry_to_symbol_table($2->entry);
    }
    ;

tfpdef
    : NAME {
        $$ = create_node("TFPDEF");
        struct node_ast* node_1 = create_node_lex("NAME", $1);
        $$->children.push_back(node_1);
        
        
        if(strcmp($1, "self")==0){
            // point to the
            $$->entry = create_table_entry($1, table_stack.top()->parent->name, NULL, yylineno, "NAME", offset);
        //if the name is self, them allot it type of the parent
            offset += 8;//size of pointer
           
        }else{
            // cerr<<"CANNOT HAVE A VARIABLE NAMED "<<$1<<" IN FUNCTION PARAMETERS WITHOUT TYPE"<<endl;
            exit(1);
        }
        
        // add_entry_to_symbol_table($$->entry);

    }
    | NAME COLON atom_expr {
        $$ = create_node("TFPDEF");
        struct node_ast* node_1 = create_node_lex("NAME", $1);
        $$->children.push_back(node_1);
        $$->children.push_back($3);
        $$->entry = create_table_entry($1, $3->type, NULL, yylineno, "NAME", offset);
        //PRINT THE CHILDREN

        if($3->children.size() == 2){
            
            simplify_tree($3);
            // in case of array
            $$->entry->is_array = true;
            // cerr<<"array type is"<<$3->children[1]->name<<endl;
            $$->entry->array_type = $3->children[1]->type ;
            // cerr<<"reached here"<<endl;

            offset +=8;
        }
        // cerr<<"reached here"<<endl;

        offset += type_size[$3->type];
        
        // add_entry_to_symbol_table($$->entry);

    
    }
    ;

stmt
    : simple_stmt {
        $$ = create_node("STMT");
        $$->children.push_back($1);
        // add all entries of simple_stmt to entries of stmt
        $$->entries = $1->entries;
    }
    | compound_stmt {
        $$ = create_node("STMT");
        $$->children.push_back($1);
        $$->entries = $1->entries;
    }
    ;

simple_stmt
    : small_stmt simple_stmt_lines NEWLINE {
        $$ = create_node("SIMPLE_STMT");
        $$->children.push_back($1);
       

        // add all entries of small_stmt to entries of simple_stmt
        for (int i = 0; i < $1->entries.size(); i++){
            add_entry_to_entries($$->entries, $1->entries[i]);
        }
        for (int i = 0; i < $2->entries.size(); i++){
            add_entry_to_entries($$->entries, $2->entries[i]);
        }
        
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        free($2);

    }
    | small_stmt simple_stmt_lines SEMICOLON NEWLINE {
        $$ = create_node("SIMPLE_STMT");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        free($2);
    }
    ;

simple_stmt_lines
    : {
        $$ = create_node("EMPTY_PROD");
    }
    | simple_stmt_lines simple_stmt_line{
        $$ = create_node("SIMPLE_STMT_LINES");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        free($1);
        $$->children.push_back($2);
        $$->entries = $1->entries;
        for (int i = 0; i < $2->entries.size(); i++){
            add_entry_to_entries($$->entries, $2->entries[i]);
        }
    }
    ;

simple_stmt_line
    : SEMICOLON small_stmt {
        $$ = create_node("SIMPLE_STMT_LINE");
        $$->children.push_back(create_node("SEMICOLON"));
        $$->children.push_back($2);

        // add all entries of small_stmt to entries of simple_stmt_line
        $$->entries = $2->entries;
    }
    ;

small_stmt
    : expr_stmt {
        $$ = create_node("SMALL_STMT");
        $$->children.push_back($1);
        // add all entries of expr_stmt to entries of small_stmt
        $$->entries = $1->entries;
        
    }
    | del_stmt {
        $$ = create_node("SMALL_STMT");
        $$->children.push_back($1);
        //TODO : check if the statement to be deleted exists
    }
    | pass_stmt {
        $$ = create_node("SMALL_STMT");
        $$->children.push_back($1);
    }
    | flow_stmt { // break, continue, return
        $$ = create_node("SMALL_STMT");
        $$->children.push_back($1);
    }
    | assert_stmt{
        $$ = create_node("SMALL_STMT");
        $$->children.push_back($1);
    }
    | global_stmt{
        $$ = create_node("SMALL_STMT");
        $$->children.push_back($1);
        //TODO : stuff to be handled here
    }
    ;

global_stmt
    : GLOBAL NAME {
        $$ = create_node("GLOBAL_STMT");
        struct node_ast* node_1 = create_node("GLOBAL");
        $$->children.push_back(node_1);
        struct node_ast* node_2 = create_node_lex("NAME", $2);
        $$->children.push_back(node_2);
        //TODO: stuff is to be handled here

        $$->entry = symbol_table_lookup($2);
        if($$->entry == NULL){
            error_report("Syntax Error: Variable not defined", yylineno);
        }
        add_entry_to_entries(global_table->entries, $$->entry);
        //we also need to remove from the current scope
    
        

        //deal with global entries
        // handle_global_entries($$->entries);
        
    }
    ;



expr_stmt
    : test{
        $$ = create_node("EXPR_STMT");
        simplify_tree($1);
        $$->children.push_back($1);
    }
    | atom_expr EQUAL test { 
        //changed from testlist to test
        $$ = create_node("EXPR_STMT");
        simplify_tree($1);
        simplify_tree($3);
        
        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        
        if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
            }
        }

        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);

            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }

        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        // else if($1->name == "FUNC_CALL"){
        //     $1->type = get_return_type($1->children[0]->lexval);
        //     $1->type_set = true;
        //     $1->lexval = $1->children[0]->lexval;
        // }
        
        if(!check_equal_to_expr($3->type, $1->type)){
            error_report("Syntax Error: Invalid operation", yylineno);
        }
        $$->children.push_back($1);
        $$->children.push_back(create_node("EQUAL"));
        $$->children.push_back($3);
     
        struct node_ast* node = parse_tree_to_ast_expr_right_ass($$->children);
        $$->children.clear();
        $$->children.push_back(node);
    }
    | atom_expr COLON atom_expr {
        $$ = create_node("EXPR_STMT");
        $$->children.push_back($1);
        simplify_tree($1);
        simplify_tree($3);
        //get last child of $$ and add $3 to that child
        $1->children[$$->children.size()-1]->children.push_back($1);
        // create a new entry here :
        $3->entry = create_table_entry($1->lexval, $3->type, NULL, yylineno, "NAME", offset);
        
        //check if it is of class type
        if(get_obj_class($3->lexval)){
             $3->is_object = true;
            $3->symbol_t = get_obj_class($3->lexval);
            // cerr<<"$3->name"<<$3->lexval<<endl;   
        }

        

        offset += type_size[$3->type];
        add_entry_to_symbol_table($3->entry);
        add_entry_to_entries($$->entries, $3->entry);
    }
    | atom_expr COLON atom_expr EQUAL test {
        $$ = create_node("EXPR_STMT");
        struct node_ast* node = create_node("EQUAL");
        $$->children.push_back(node);
        node->children.push_back($1);
        simplify_tree($1);
        node->children.push_back($5);
        simplify_tree($3);
        if($1->children.size()==0){
            $1->children.push_back($3);
        }else{
            $1->children[$1->children.size()-1]->children.push_back($3);
        }
        // cerr<<"$1->lexval"<<$1->lexval<<endl;
        // cerr<<"$1->is_dot"<<$1->is_dot<<endl;
        // cerr<<"$1->name"<<$1->name<<endl;
        $3->entry = create_table_entry($1->lexval, $3->type, NULL, yylineno, "NAME", offset);

        //CHECK IF IT IS A CLASS TYPE
        if(get_obj_class($3->lexval)){

            //this is of object type
            $3->is_object = true;
            $3->symbol_t = get_obj_class($3->lexval);
            // cerr<<"$3->name"<<$3->lexval<<endl;
        }
        if($3->children.size() == 2){
            //this is of array type
            $3->entry->is_array = true;
            simplify_tree($5);
            simplify_tree($3);
            //number of elements in the array
            int num_elements = $5->children.size();
            // cout << "AHAHAHAHA : " << $3->children[1]->type << endl;
            $3->entry->array_type = $3->children[1]->type;
            $3->entry->array_size = num_elements;
            array_num_elements[$1->lexval]=num_elements;
            offset += type_size[$3->entry->array_type]*num_elements;
            $5->entry= $3->entry; //for testlistcomp

        }else{
            offset += type_size[$3->type];
        }

        
        
        add_entry_to_symbol_table($3->entry);
        
        // if($3->entry->name.length() >= 4){
            
        //     if($3->entry->name.substr(0,4) == "self"){

        //         // add_to_parent_table($3->entry);
        //     }
        // }
        add_entry_to_entries($$->entries, $3->entry);
    }
    | test augassign test {
        $$ = $2->children[0];
        simplify_tree($1);
        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
            
        }
        else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);               
                $1->type_set = true;
            }
        }
       

        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }

        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }

        $$->children.push_back($1);
        $$->children.push_back($3);

        if(!check_augassign_merge($1->type, $3->type)){
            error_report("Syntax Error: Invalid operation", yylineno);
        }
        free($2);



    }
    ;


augassign
    :PLUSEQUAL {
        $$ = create_node("AUGASSIGN");
        struct node_ast* node_1 = create_node("PLUSEQUAL");
        $$->children.push_back(node_1);
    }
    | MINUSEQUAL {
        $$ = create_node("AUGASSIGN");
        struct node_ast* node_1 = create_node("MINUSEQUAL");
        $$->children.push_back(node_1);
    }
    | STAREQUAL {
        $$ = create_node("AUGASSIGN");
        struct node_ast* node_1 = create_node("MULEQUAL");
        $$->children.push_back(node_1);
    }
    | SLASHEQUAL {
        $$ = create_node("AUGASSIGN");
        struct node_ast* node_1 = create_node("DIVEQUAL");
        $$->children.push_back(node_1);
    }
    | PERCENTEQUAL {
        $$ = create_node("AUGASSIGN");
        struct node_ast* node_1 = create_node("MODEQUAL");
        $$->children.push_back(node_1);
    }
    | AMPEREQUAL {
        $$ = create_node("AUGASSIGN");
        struct node_ast* node_1 = create_node("AMPEREQUAL");
        $$->children.push_back(node_1);
    }
    | PIPEEQUAL {
        $$ = create_node("AUGASSIGN");
        struct node_ast* node_1 = create_node("OREQUAL");
        $$->children.push_back(node_1);
    }
    | CARETEQUAL {
        $$ = create_node("AUGASSIGN");
        struct node_ast* node_1 = create_node("XOREQUAL");
        $$->children.push_back(node_1);
    }
    | LSHIFTEQUAL {
        $$ = create_node("AUGASSIGN");
        struct node_ast* node_1 = create_node("LSHIFTEQUAL");
        $$->children.push_back(node_1);
    }
    | RSHIFTEQUAL {
        $$ = create_node("AUGASSIGN");
        struct node_ast* node_1 = create_node("RSHIFTEQUAL");
        $$->children.push_back(node_1);
    }
    |FLOORDIV_ASSIGN {
        $$ = create_node("AUGASSIGN");
        struct node_ast* node_1 = create_node("FLOORDIV_ASSIGN");
        $$->children.push_back(node_1);
    }
    ;


del_stmt
    : DEL atom_expr {
        $$ = create_node("DEL_STMT");
        struct node_ast* node_1 = create_node("DEL");
        $$->children.push_back(node_1);
        $$->children.push_back($2);
    }
    ;
pass_stmt
    : PASS {
        $$ = create_node("PASS_STMT");
        struct node_ast* node_1 = create_node("PASS");
        $$->children.push_back(node_1);
    }
    ;
flow_stmt
    : break_stmt {
        $$ = create_node("FLOW_STMT");
        $$->children.push_back($1);
    }
    | continue_stmt {
        $$ = create_node("FLOW_STMT");
        $$->children.push_back($1);
    }
    | return_stmt {
        $$ = create_node("FLOW_STMT");
        $$->children.push_back($1);
    }
    ;
break_stmt
    : BREAK {
        $$ = create_node("BREAK");
        
        node_ast* loop_node = LOCAL_TEMP.top();
        stack<node_ast*> temp;
        while(loop_node->name != "WHILE_STMT" && loop_node->name != "FOR_STMT"){
            temp.push(loop_node);
            LOCAL_TEMP.pop();
            loop_node = LOCAL_TEMP.top();
        }

        $$->label_next = loop_node->label_next;
        while(!temp.empty()){
            LOCAL_TEMP.push(temp.top());
            temp.pop();
        }

    }
    ;
continue_stmt
    : CONTINUE {
        $$ = create_node("CONTINUE");
        node_ast* loop_node = LOCAL_TEMP.top();
        stack<node_ast*> temp;
        while(loop_node->name != "WHILE_STMT" && loop_node->name != "FOR_STMT"){
            temp.push(loop_node);
            LOCAL_TEMP.pop();
            loop_node = LOCAL_TEMP.top();
        }
        if(loop_node->name == "WHILE_STMT"){
            $$->label_next = loop_node->label_begin;

        }else if(loop_node->name == "FOR_STMT"){
            $$->label_next = loop_node->label_iter;
            // i NEED TO INCREMENT THE ITERATOR HERE AS WELL

        }

        while(!temp.empty()){
            LOCAL_TEMP.push(temp.top());
            temp.pop();
        }
        
    }
    ;
return_stmt
    : RETURN {
        $$ = create_node("RETURN_STMT");
        struct node_ast* node_1 = create_node("RETURN");
        $$->children.push_back(node_1);


    }
    | RETURN testlist {
        $$ = create_node("RETURN_STMT");
        struct node_ast* node_1 = create_node("RETURN");
        simplify_tree($2);
        if($2->name == "NAME"){
            if($2->is_dot){
                //continue
                // $2->type = get_dot_type($2->dot_obj, $2->dot_name);
            }
            else if(symbol_table_lookup($2->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
           
        }else if($2->name == "ATOM_EXPR"){
            if($2->children.size() == 2){
                $2->type = get_array_ref_type($2->children[0]->lexval, $2->children[1]);
                $2->array_entry = symbol_table_lookup($2->children[0]->lexval);
                $2->type_set = true;
            }
        }else if($2->name == "FUNC_CALL"){
            if($2->is_func_dot){
                $2->type = get_return_type_dot($2->func_dot_obj, $2->func_dot_name);
                $2->type_set = true;
                $2->lexval = $2->func_dot_obj + "." + $2->func_dot_name;

            }else{
                $2->type = get_return_type($2->children[0]->lexval);
                $2->type_set = true;
                $2->lexval = $2->children[0]->lexval;
            }
        }else if($2->name == "TESTLIST_COMP" && $2->children.size() == 1){
            $2 = $2->children[0];
        }
        $$->children.push_back(node_1);
        $$->children.push_back($2);

        // verify_return_type($2->type, FUNC_NODE_TEMP->symbol_t->return_type);
    }
    ;

assert_stmt
    : ASSERT test {
        $$ = create_node("ASSERT_STMT");
        struct node_ast* node_1 = create_node("ASSERT");
        $$->children.push_back(node_1);
        $$->children.push_back($2);
    }
    | ASSERT test COMMA test {
        $$ = create_node("ASSERT_STMT");
        struct node_ast* node_1 = create_node("ASSERT");
        $$->children.push_back(node_1);
        $$->children.push_back($2);
        $$->children.push_back($4);
    }
    ;

compound_stmt
    : if_stmt  {
        $$ = create_node("COMPOUND_STMT");
        $$->children.push_back($1);
        $$->entries = $1->entries;

    }
    | while_stmt {
        $$ = create_node("COMPOUND_STMT");
        $$->children.push_back($1);
        $$->entries = $1->entries;
    }
    | for_stmt {
        $$ = create_node("COMPOUND_STMT");
        $$->children.push_back($1);
        $$->entries = $1->entries;
    }
    | funcdef {
        $$ = create_node("COMPOUND_STMT");
        $$->children.push_back($1);
        table_stack.pop();
        // add_symbol_table_to_class_entries($$->entries,$1->symbol_t);
    }
    | classdef {
        $$ = create_node("COMPOUND_STMT");
        $$->children.push_back($1);
        table_stack.pop();
    }
    ;

if_stmt
    : IF test {
        $<node_ptr>$ = create_node("IF_STMT");
        $<node_ptr>$->symbol_t = create_symbol_table("IF");
        table_stack.push($<node_ptr>$->symbol_t);
        LOCAL_TEMP.push($<node_ptr>$);
        $<node_ptr>$->label_next = create_new_label();//if_stmt next label
        
        simplify_tree($2);
        if($2->name == "TESTLIST_COMP" && $2->children.size() == 1){
            $2 = $2->children[0]; 
        }
    }COLON suite{
        $<node_ptr>$ = LOCAL_TEMP.top();
        $<node_ptr>$->symbol_t = table_stack.top();
        simplify_tree($2);
        $2->label_true = create_new_label();//create new true label  B.true
        $<node_ptr>$->children.push_back($2);
        struct node_ast* node = create_node("THEN");//this is for the suite part
        $<node_ptr>$->children.push_back(node);
        node->children.push_back($5);
        
        $<node_ptr>$->entries = $5->entries;
        add_scope_entries_to_parent();
        table_stack.pop();

        
    } if_stmt_line {
        $<node_ptr>$ = LOCAL_TEMP.top();
        $<node_ptr>$->symbol_t = table_stack.top();
        $<node_ptr>$->children.push_back($7);
        for(int i = 0; i < $7->entries.size(); i++){
            add_entry_to_entries_union($<node_ptr>$->entries, $7->entries[i]);
        }

    }else_block{
        $$ = LOCAL_TEMP.top();
        LOCAL_TEMP.pop();
        $$->symbol_t = table_stack.top();


       
        
        
        $$->children.push_back($9);
        for(int i = 0; i < $9->entries.size(); i++){
            add_entry_to_entries_union($$->entries, $9->entries[i]);
        }
        simplify_tree($$);
        handle_label_false_elif_block($$);
        handle_inherited_attributes($2);
        

    }
    ;

else_block
    :{
        $$ = create_node("EMPTY_PROD");
        $$->entries = vector<table_entry*>();
    }
    | ELSE{
        $<node_ptr>$ = create_node("ELSE_BLOCK");
        $<node_ptr>$->symbol_t = create_symbol_table("ELSE");
        table_stack.push($<node_ptr>$->symbol_t);
        LOCAL_TEMP.push($<node_ptr>$);

    } COLON suite {
        $$ = LOCAL_TEMP.top();
        LOCAL_TEMP.pop();
        $$->children.push_back($4);
        $$->entries = $4->entries;
        add_scope_entries_to_parent();
        table_stack.pop();
    }
    
if_stmt_line
    : {
        $$ = create_node("EMPTY_PROD");
        $$->entries = vector<table_entry*>();
    }
    | if_stmt_line ELIF{
        $<node_ptr>$ = create_node("ELIF_BLOCK");
        $<node_ptr>$->symbol_t = create_symbol_table("ELIF");
        table_stack.push($<node_ptr>$->symbol_t);
        LOCAL_TEMP.push($<node_ptr>$);
    } test COLON suite {
        $<node_ptr>$ = LOCAL_TEMP.top();
        LOCAL_TEMP.pop();
        $$->symbol_t = table_stack.top();
        simplify_tree($4);
        if($4->name == "TESTLIST_COMP" && $4->children.size() == 1){
            // cerr<<"Inside testlist comp"<<endl;
            $4 = $4->children[0];
        }
        $4->label_true = create_new_label();//create new true label  B.true for elif statements
        $$->children.push_back($4);
        $$->children.push_back($6);
        $$->children.push_back($1);
        $$->entries = $6->entries;
        for(int i = 0; i < $1->entries.size(); i++){
            add_entry_to_entries_union($$->entries, $1->entries[i]);
        }
       
        
        // cerr<<"Inside elif block"<<$4->name<<endl;

        add_scope_entries_to_parent();
        
        table_stack.pop();
        handle_inherited_attributes($4);
        
    }
    ;

while_stmt
    : WHILE{
        $<node_ptr>$ = create_node("WHILE_STMT");
        $<node_ptr>$->symbol_t = create_symbol_table("WHILE");
        $<node_ptr>$->label_next = create_new_label();
        $<node_ptr>$->label_begin = create_new_label();
        table_stack.push($<node_ptr>$->symbol_t);
        LOCAL_TEMP.push($<node_ptr>$);
    } test COLON suite {
        $<node_ptr>$ = LOCAL_TEMP.top();
        // $<node_ptr>$->children.push_back(create_node("WHILE"));
        simplify_tree($3);

        if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        $<node_ptr>$->children.push_back($3);
        $<node_ptr>$->children.push_back($5);
        $<node_ptr>$->entries = $5->entries;
        add_scope_entries_to_parent();
        table_stack.pop();

        
        
        $3->label_true = create_new_label();
        $3->label_false = $<node_ptr>$->label_next;

        simplify_tree($5);
        $5->label_next = $<node_ptr>$->label_begin;
        handle_inherited_attributes($3);
    }else_block{
        $$ = LOCAL_TEMP.top();
        LOCAL_TEMP.pop();
        $$->children.push_back($7);
        for(int i = 0; i < $7->entries.size(); i++){
            add_entry_to_entries_union($$->entries, $7->entries[i]);
        }
        
        
    }
    ;

for_stmt
    : FOR{
        $<node_ptr>$ = create_node("FOR_STMT");
        $<node_ptr>$->symbol_t = create_symbol_table("FOR");

        $<node_ptr>$->label_next = create_new_label(); //in P->S will be used later in break
        $<node_ptr>$->label_begin = create_new_label(); //will be used later in cont
        $<node_ptr>$->label_iter = create_new_label();

        table_stack.push($<node_ptr>$->symbol_t);
        LOCAL_TEMP.push($<node_ptr>$);
        
    } NAME IN testlist COLON suite {
        $<node_ptr>$ = LOCAL_TEMP.top();
        $<node_ptr>$->children.push_back(create_node("FOR"));
        $<node_ptr>$->children.push_back(create_node_lex("NAME", $3));
       
        table_entry* name_entry = symbol_table_lookup($3);
        if(name_entry == NULL){
            error_report("Syntax Error: Variable not defined", yylineno);
        }

        $<node_ptr>$->children.push_back(create_node("IN"));
        $<node_ptr>$->children.push_back($5);
        $<node_ptr>$->children.push_back($7);
        $<node_ptr>$->entries = $7->entries;
        add_scope_entries_to_parent();
        table_stack.pop();

    }else_block{
        $$ = LOCAL_TEMP.top();
        LOCAL_TEMP.pop();
        $$->children.push_back($9);
        for(int i = 0; i < $9->entries.size(); i++){
            add_entry_to_entries_union($<node_ptr>$->entries, $9->entries[i]);
        }
        simplify_tree($$);
       

        $5->label_true = create_new_label(); //for tej
        
    };


suite
    : simple_stmt {
        $$ = create_node("SUITE");
        $$->children.push_back($1);

        // add all entries of simple_stmt to entries of suite

        $$->entries = $1->entries;
    }
    | NEWLINE TOKEN_INDENT stmts TOKEN_DEDENT {
        $$ = create_node("SUITE");
        for(int i = 0; i < $3->children.size(); i++){
            $$->children.push_back($3->children[i]);
        }
        $$->entries = $3->entries;
        free($3);
    }
    | NEWLINE stmts TOKEN_DEDENT {
        error_report("Syntax Error: Indentation Error", yylineno);
    }
    ;

stmts
    : stmts stmt {
        $$ = create_node("STMS");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        $$->entries = $1->entries;
        for (int i = 0; i < $2->entries.size(); i++){
            add_entry_to_entries($$->entries, $2->entries[i]);
        }

        free($1);
        $$->children.push_back($2);
    }
    | stmt {
        $$ = create_node("STMS");
        $$->children.push_back($1);
        $$->entries = $1->entries;
    }
    ;

test
    : or_test {
        $$ = create_node("TEST");
        $$->children.push_back($1);
        if($1->type_set){
            $$->type = $1->type;
            $$->type_set = $1->type_set;
        }

    }
    ;


or_test
    : and_test or_test_line {
        $$ = create_node("OR_TEST");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        simplify_tree($1);
        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        
        else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
            }
        }else if($1->name == "FUNC_CALL"){
            if($1->is_func_dot){
                $1->type = get_return_type_dot($1->func_dot_obj, $1->func_dot_name);
                $1->type_set = true;
                $1->lexval = $1->func_dot_obj + "." + $1->func_dot_name;
            }else{
                $1->type = get_return_type($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval;
            }
        }else if($1->name == "TESTLIST_COMP" && $1->children.size() == 1){
            $1 = $1->children[0];
        }
        
        $$->type = merge_and_test_types($2->type, $1->type);// same semantics as and
        $$->type_set = true;
        // $1->label_false = create_new_label();

        free($2);


    }
    ;

or_test_line
    : {
        $$ = create_node("EMPTY_PROD");
        $$->type = "NOTDEFINED";
    }
    | or_test_line OR and_test {
        $$ = create_node("OR_TEST_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }

        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }

        $$->type = merge_and_test_types($1->type, $3->type);
        $$->type_set = true;

        free($1);
        $$->children.push_back(create_node("OR"));
        $$->children.push_back($3);
    }
    ;

and_test
    : not_test and_test_line {
        $$ = create_node("AND_TEST");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        simplify_tree($1);
        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
            }
        }
        else if($1->name == "FUNC_CALL"){
            if($1->is_func_dot){
                $1->type = get_return_type_dot($1->func_dot_obj, $1->func_dot_name);
                $1->type_set = true;
                $1->lexval = $1->func_dot_obj + "." + $1->func_dot_name;
            }else{
                $1->type = get_return_type($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval;
            }
        }else if($1->name == "TESTLIST_COMP" && $1->children.size() == 1){
            $1 = $1->children[0];
        }

        $$->type = merge_and_test_types($2->type, $1->type);
        $$->type_set = true;
        free($2);
        
    }
    ;

and_test_line
    : {
        $$ = create_node("EMPTY_PROD");
        $$->type = "NOTDEFINED";
    }
    | and_test_line AND not_test {
        $$ = create_node("AND_TEST_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        $$->type = merge_and_test_types($1->type, $3->type);
        $$->type_set = true;
        $$->children.push_back(create_node("AND"));
        $$->children.push_back($3);
        free($1);

    }
    ;

not_test
    : NOT not_test {
        $$ = create_node("NOT_TEST");
        struct node_ast* node_1 = create_node("NOT");
        simplify_tree($2);
        if($2->name == "NAME"){
            if($2->is_dot){
                //continue
                // $2->type = get_dot_type($2->dot_obj, $2->dot_name);
            }
            else if(symbol_table_lookup($2->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($2->name == "ATOM_EXPR"){
            if($2->children.size() == 2){
                $2->type = get_array_ref_type($2->children[0]->lexval, ($2->children[1]));
                $2->array_entry = symbol_table_lookup($2->children[0]->lexval);
                $2->type_set = true;
            }
        }else if($2->name == "FUNC_CALL"){
            if($2->is_func_dot){
                $2->type = get_return_type_dot($2->func_dot_obj, $2->func_dot_name);
                $2->type_set = true;
                $2->lexval = $2->func_dot_obj + "." + $2->func_dot_name;
            }else{
                $2->type = get_return_type($2->children[0]->lexval);
                $2->type_set = true;
                $2->lexval = $2->children[0]->lexval;
            }

        }else if($2->name == "TESTLIST_COMP" && $2->children.size() == 1){
            $2 = $2->children[0];
        }
        $$->type = merge_not_test_types($2->type);
        $$->type_set = true;
        
        $$->children.push_back(node_1);
        $$->children.push_back($2);
    }
    | comparison {
        $$ = create_node("NOT_TEST");
        $$->children.push_back($1);

        if($1->type_set){
            $$->type = $1->type;
            $$->type_set = true;
        }
    }

    ;

comparison
    : expr comp_op expr {
        $$ = create_node("COMPARISON");
        simplify_tree($1);
        simplify_tree($3);
        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
            }
        }else if($1->name == "FUNC_CALL"){
            if($1->is_func_dot){
                $1->type = get_return_type_dot($1->func_dot_obj, $1->func_dot_name);
                $1->type_set = true;
                $1->lexval = $1->func_dot_obj + "." + $1->func_dot_name;
            }else{
                $1->type = get_return_type($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval;
            }
        }else if($1->name == "TESTLIST_COMP" && $1->children.size() == 1){
            $1 = $1->children[0];
        }
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }

        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;
            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        $$->type = merge_comp_types($1->type, $3->type);
        $$->type_set = true;

        if(($1->type == "STR"||$1->type == "STRING") && ($3->type == "STR"|| $3->type == "STRING")){
            // cerr<<"STRING RELATIONAL ENCOUNTERED"<<endl;
            $$->compare_type_str = true;
        }

        $$->children.push_back($1);
        $$->children.push_back($2);
        $$->children.push_back($3);
    }
    | expr {
        $$ = create_node("COMPARISON");
        $$->children.push_back($1);
        if($1->type_set){
            $$->type = $1->type;
            $$->type_set = true;
        }
    }
    ;

//we will only support single comparison for now

// comparison_line
//     : {
//         $$ = create_node("EMPTY_PROD");
//         $$->type = "NOTDEFINED";
//     }
//     | comparison_line comp_op expr {
//         $$ = create_node("COMPARISON_LINE");
//         for(int i = 0; i < $1->children.size(); i++){
//             $$->children.push_back($1->children[i]);
//         }
//         simplify_tree($3);
//         if($3->name == "NAME"){
//             if(symbol_table_lookup($3->lexval) == NULL){
//                 error_report("Syntax Error: Variable not defined", yylineno);
//             }
//         }

//         $$->type = merge_comp_types($1->type, $3->type);
//         $$->children.push_back($2);
//         $$->children.push_back($3);
//         free($1);
//     }
//     ;

comp_op
    :LT {
        $$ = create_node("LT");

    }
    |GT {
        $$ = create_node("GT");
    }
    |EQ {
        $$ = create_node("EQ");
    }
    |GE {
        $$ = create_node("GE");
    }
    |LE {
        $$ = create_node("LE");
    }
    |NE {
        $$ = create_node("NE");
    }
    |IN {
        $$ = create_node("IN");
    }
    |NOT IN {
        $$ = create_node("NOT IN");
    }
    |IS {
        $$ = create_node("IS");
    }
    |IS NOT {
        $$ = create_node("IS NOT");
    }
    ;


expr
    : xor_expr expr_line {
        $$ = create_node("EXPR");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        simplify_tree($1);
        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
            }
        }else if($1->name == "FUNC_CALL"){
            if($1->is_func_dot){
                $1->type = get_return_type_dot($1->func_dot_obj, $1->func_dot_name);
                $1->type_set = true;
                $1->lexval = $1->func_dot_obj + "." + $1->func_dot_name;
            }else{
                $1->type = get_return_type($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval;
            }
        }else if($1->name == "TESTLIST_COMP" && $1->children.size() == 1){
            $1 = $1->children[0];
        }
        $$->type = merge_and_expr_types($2->type, $1->type);
        $$->type_set = true;
        // simplify_tree($$);
        // if($$->children.size()>2 && $$->children[2]->name == "PIPE"){
        //     $$->name == "OR_EXPR";
        // }

        free($2);

        
    }
    ;

expr_line
    : {
        $$ = create_node("EMPTY_PROD");
        $$->type = "NOTDEFINED";

    }
    | expr_line PIPE xor_expr {
        $$ = create_node("EXPR_LINE");
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
           if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;
            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        $$->type = merge_and_expr_types($1->type, $3->type);
        $$->type_set = true;
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }


        $$->children.push_back(create_node("PIPE"));
        $$->children.push_back($3);
        free($1);

    }
    ;

xor_expr
    : and_expr xor_expr_line {
        $$ = create_node("XOR_EXPR");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        simplify_tree($1);
        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
            }
        }else if($1->name == "FUNC_CALL"){
            if($1->is_func_dot){
                $1->type = get_return_type_dot($1->func_dot_obj, $1->func_dot_name);
                $1->type_set = true;
                $1->lexval = $1->func_dot_obj + "." + $1->func_dot_name;
            }else{
                $1->type = get_return_type($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval;
            }
        }else if($1->name == "TESTLIST_COMP" && $1->children.size() == 1){
            $1 = $1->children[0];
        }
        $$->type = merge_and_expr_types($2->type, $1->type);
        $$->type_set = true;
        free($2);

        
    }
    ;

xor_expr_line
    : {
        $$ = create_node("EMPTY_PROD");
        $$->type = "NOTDEFINED";
    }
    | xor_expr_line CARET and_expr{
        $$ = create_node("XOR_EXPR_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }

        $$->type = merge_and_expr_types($1->type, $3->type);
        $$->children.push_back(create_node("CARET"));
        $$->children.push_back($3);
        free($1);

    }
    ;

and_expr
    : shift_expr and_expr_line {
        $$ = create_node("AND_EXPR");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        simplify_tree($1);
        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
            }
        }else if($1->name == "FUNC_CALL"){
            if($1->is_func_dot){
                $1->type = get_return_type_dot($1->func_dot_obj, $1->func_dot_name);
                $1->type_set = true;
                $1->lexval = $1->func_dot_obj + "." + $1->func_dot_name;
            }else{
                $1->type = get_return_type($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval;
            }
        }else if($1->name == "TESTLIST_COMP" && $1->children.size() == 1){
            $1 = $1->children[0];
        }
        $$->type = merge_and_expr_types($2->type, $1->type);
        $$->type_set = true;
        free($2);
    }
    ;

and_expr_line
    : {
        $$ = create_node("EMPTY_PROD");
        $$->type = "NOTDEFINED";
    }
    | and_expr_line AMPER shift_expr {
        $$ = create_node("AND_EXPR_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        $$->type = merge_and_expr_types($1->type, $3->type);
        $$->type_set = true;

        $$->children.push_back(create_node("AMPER"));
        $$->children.push_back($3);
        free($1);

    }
    ;

shift_expr
    : arith_expr shift_expr_line {
        $$ = create_node("SHIFT_EXPR");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        simplify_tree($1);
        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
            }
        }else if($1->name == "FUNC_CALL"){
            if($1->is_func_dot){
                $1->type = get_return_type_dot($1->func_dot_obj, $1->func_dot_name);
                $1->type_set = true;
                $1->lexval = $1->func_dot_obj + "." + $1->func_dot_name;
            }else{
                $1->type = get_return_type($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval;
            }
        }else if($1->name == "TESTLIST_COMP" && $1->children.size() == 1){
            $1 = $1->children[0];
        }
        $$->type = merge_shift_expr_types($2->type, $1->type);
        $$->type_set = true;

        // if($1->type_set){
        //     $$->type = $1->type;
        //     $$->type_set = true;
        // }
        struct node_ast* node = parse_tree_to_ast_expr_left_ass($$->children);
        $$->children.clear();
        node->type = $$->type;
        node->type_set = true;
        $$->children.push_back(node);
        // free($2);

    }
    ;

shift_expr_line
    : {
        $$ = create_node("EMPTY_PROD");
        $$->type = "NOTDEFINED";
    }
    | shift_expr_line LSHIFT arith_expr {
        $$ = create_node("SHIFT_EXPR_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;
            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        $$->type = merge_shift_expr_types($1->type, $3->type);
        $$->type_set = true;

        $$->children.push_back(create_node("LSHIFT"));
        $$->children.push_back($3);
        free($1);

    }
    | shift_expr_line RSHIFT arith_expr {
        $$ = create_node("SHIFT_EXPR_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;
            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        $$->type = merge_shift_expr_types($1->type, $3->type);
        $$->type_set = true;
        $$->children.push_back(create_node("RSHIFT"));
        $$->children.push_back($3);
        free($1);

    }
    ;



arith_expr
    : term arith_expr_line {
        $$ = create_node("ARITH_EXPR");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }

        simplify_tree($1);

        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
            }
        }else if($1->name == "FUNC_CALL"){
            if($1->is_func_dot){
                $1->type = get_return_type_dot($1->func_dot_obj, $1->func_dot_name);
                $1->type_set = true;
                $1->lexval = $1->func_dot_obj + "." + $1->func_dot_name;
            }else{
                $1->type = get_return_type($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval;
            }
        }else if($1->name == "TESTLIST_COMP" && $1->children.size() == 1){
            $1 = $1->children[0];
        }
        $$->type = merge_arith_expr_types($2->type, $1->type);
        $$->type_set = true;
        struct node_ast* node = parse_tree_to_ast_expr_left_ass($$->children);
        $$->children.clear();

        
        node->type = $$->type;
        node->type_set = true;
        $$->children.push_back(node);
        

        free($2);

    }
    ;

arith_expr_line
    : {
        $$ = create_node("EMPTY_PROD");
        $$->type = "NOTDEFINED";
    }
    | arith_expr_line PLUS term {
        $$ = create_node("ARITH_EXPR_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        $$->children.push_back(create_node("PLUS"));
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        $$->type = merge_arith_expr_types($1->type, $3->type);
        $$->children.push_back($3);
        free($1);

    }
    | arith_expr_line MINUS term {
        $$ = create_node("ARITH_EXPR_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        $$->children.push_back(create_node("MINUS"));
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        $$->type = merge_arith_expr_types($1->type, $3->type);
        $$->type_set = true;
        $$->children.push_back($3);
        free($1);

    }
    ;

term
    : factor term_line {
        $$ = create_node("TERM");
        simplify_tree($1);
        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
            }
        }else if($1->name == "FUNC_CALL"){
            if($1->is_func_dot){
                $1->type = get_return_type_dot($1->func_dot_obj, $1->func_dot_name);
                $1->type_set = true;
                $1->lexval = $1->func_dot_obj + "." + $1->func_dot_name;
            }else{
                $1->type = get_return_type($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval;
            }
        }else if($1->name == "TESTLIST_COMP" && $1->children.size() == 1){
            $1 = $1->children[0];
        }

        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        struct node_ast* node = parse_tree_to_ast_expr_left_ass($$->children);
        $$->children.clear();
        $$->children.push_back(node);

        $$->type = merge_term($1->type, $2->type);
        $$->type_set = true;
        free($2);

    
    }
    ;

term_line
    : {
        $$ = create_node("EMPTY_PROD");
        $$->type = "NOTDEFINED";

    }
    | term_line STAR factor {
        $$ = create_node("TERM_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        $$->children.push_back(create_node("MULT"));
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        $$->children.push_back($3);

        $$->type = merge_term($1->type, $3->type);
        $$->type_set = true;
        free($1);
    }
    | term_line AT factor {
        $$ = create_node("TERM_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        $$->children.push_back(create_node("AT"));
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        
        $$->children.push_back($3);
        $$->type = merge_term($1->type, $3->type);
        $$->type_set= true;
        free($1);


    }
    | term_line SLASH factor {
        $$ = create_node("TERM_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        $$->children.push_back(create_node("DIV"));
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }


        $$->children.push_back($3);
        $$->type = merge_term($1->type, $3->type);
        $$->type_set = true;

        free($1);

    }
    | term_line PERCENT factor {
        $$ = create_node("TERM_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        $$->children.push_back(create_node("MOD"));
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }

        $$->children.push_back($3);

        $$->type = merge_term($1->type, $3->type);
        $$->type_set = true;
        free($1);

    }
    | term_line DOUBLESLASH factor {
        $$ = create_node("TERM_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        $$->children.push_back(create_node("DOUBLESLASH"));
        simplify_tree($3);
        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;

            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }
        $$->children.push_back($3);
        $$->type = merge_term($1->type, $3->type);
        free($1);

    }
    ;

factor
    : PLUS factor {
        $$ = create_node("FACTOR");
        struct node_ast* node_1 = create_node("PLUS");
        $$->children.push_back(node_1);
        simplify_tree($2);
        if($2->name == "NAME"){
            if($2->is_dot){
                //continue
                // $2->type = get_dot_type($2->dot_obj, $2->dot_name);
            }
            else if(symbol_table_lookup($2->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($2->name == "ATOM_EXPR"){
            if($2->children.size() == 2){
                $2->type = get_array_ref_type($2->children[0]->lexval, ($2->children[1]));
                $2->array_entry = symbol_table_lookup($2->children[0]->lexval);
                $2->type_set = true;
            }
        }else if($2->name == "FUNC_CALL"){
            if($2->is_func_dot){
                $2->type = get_return_type_dot($2->func_dot_obj, $2->func_dot_name);
                $2->type_set = true;
                $2->lexval = $2->func_dot_obj + "." + $2->func_dot_name;
            }else{
                $2->type = get_return_type($2->children[0]->lexval);
                $2->type_set = true;
                $2->lexval = $2->children[0]->lexval;
            }
        }else if($2->name == "TESTLIST_COMP" && $2->children.size() == 1){
            $2 = $2->children[0];
        }
        $$->children.push_back($2);
        if($2->type_set){
            $$->type = merge_factor($2->type);
            $$->type_set = true;
        }
    }
    | MINUS factor {
        $$ = create_node("FACTOR");
        struct node_ast* node_1 = create_node("MINUS");
        $$->children.push_back(node_1);
        simplify_tree($2);
        if($2->name == "NAME"){
            if($2->is_dot){
                //continue
                // $2->type = get_dot_type($2->dot_obj, $2->dot_name);
            }
            else if(symbol_table_lookup($2->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($2->name == "ATOM_EXPR"){
            if($2->children.size() == 2){
                $2->type = get_array_ref_type($2->children[0]->lexval, ($2->children[1]));
                $2->array_entry = symbol_table_lookup($2->children[0]->lexval);
                $2->type_set = true;
            }
        }else if($2->name == "FUNC_CALL"){
            if($2->is_func_dot){
                $2->type = get_return_type_dot($2->func_dot_obj, $2->func_dot_name);
                $2->type_set = true;
                $2->lexval = $2->func_dot_obj + "." + $2->func_dot_name;
            }else{
                $2->type = get_return_type($2->children[0]->lexval);
                $2->type_set = true;
                $2->lexval = $2->children[0]->lexval;
            }
        }else if($2->name == "TESTLIST_COMP" && $2->children.size() == 1){
            $2 = $2->children[0];
        }
        $$->children.push_back($2);

        if($2->type_set){
            $$->type = merge_factor($2->type);
            $$->type_set = true;
        }
    }
    | TILDE factor {
        $$ = create_node("FACTOR");
        struct node_ast* node_1 = create_node("TILDE");
        $$->children.push_back(node_1);
        simplify_tree($2);
        if($2->name == "NAME"){
            if($2->is_dot){
                //continue
                // $2->type = get_dot_type($2->dot_obj, $2->dot_name);
            }
            else if(symbol_table_lookup($2->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($2->name == "ATOM_EXPR"){
            if($2->children.size() == 2){
                $2->type = get_array_ref_type($2->children[0]->lexval, ($2->children[1]));
                $2->array_entry = symbol_table_lookup($2->children[0]->lexval);
                $2->type_set = true;
            }
        }else if($2->name == "FUNC_CALL"){
            if($2->is_func_dot){
                $2->type = get_return_type_dot($2->func_dot_obj, $2->func_dot_name);
                $2->type_set = true;
                $2->lexval = $2->func_dot_obj + "." + $2->func_dot_name;
            }else{
                $2->type = get_return_type($2->children[0]->lexval);
                $2->type_set = true;
                $2->lexval = $2->children[0]->lexval;
            }
        }else if($2->name == "TESTLIST_COMP" && $2->children.size() == 1){
            $2 = $2->children[0];
        }
        $$->children.push_back($2);

        if($2->type_set){
            $$->type = $2->type;
            $$->type_set = true;
        }
    }
    | power {
        $$ = create_node("FACTOR");
        $$->children.push_back($1);
        if($1->type_set){
            $$->type = $1->type;
            $$->type_set = true;
        }
    }
    ;

power
    : atom_expr {
        $$ = create_node("POWER");
        $$->children.push_back($1);
        if($1->type_set){
            $$->type = $1->type;
            $$->type_set = true;
        }
    }
    | atom_expr DOUBLESTAR atom_expr {

        $$ = create_node("POWER");
        // cerr<<"Inside power"<<endl;
            
        struct node_ast* node_1 = create_node("DOUBLE_STAR");

        simplify_tree($1);
        // cerr<<"Inside power"<<endl;
        simplify_tree($3);
        node_1->children.push_back($1);
        node_1->children.push_back($3);
        if($1->name == "NAME"){
            if($1->is_dot){
                //continue
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
            }
        }else if($1->name == "FUNC_CALL"){
            if($1->is_func_dot){
                $1->type = get_return_type_dot($1->func_dot_obj, $1->func_dot_name);
                $1->type_set = true;
                $1->lexval = $1->func_dot_obj + "." + $1->func_dot_name;
            }else{
                $1->type = get_return_type($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval;
            }
        }else if($1->name == "TESTLIST_COMP" && $1->children.size() == 1){
            $1 = $1->children[0];
        }

        if($3->name == "NAME"){
            if($3->is_dot){
                //continue
                // $3->type = get_dot_type($3->dot_obj, $3->dot_name);
            }
            else if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($3->name == "ATOM_EXPR"){
            if($3->children.size() == 2){
                $3->type = get_array_ref_type($3->children[0]->lexval, $3->children[1]);
                $3->array_entry = symbol_table_lookup($3->children[0]->lexval);
                $3->type_set = true;
            }
        }else if($3->name == "FUNC_CALL"){
            if($3->is_func_dot){
                $3->type = get_return_type_dot($3->func_dot_obj, $3->func_dot_name);
                $3->type_set = true;
                $3->lexval = $3->func_dot_obj + "." + $3->func_dot_name;
            }else{
                $3->type = get_return_type($3->children[0]->lexval);
                $3->type_set = true;
                $3->lexval = $3->children[0]->lexval;
            }
        }else if($3->name == "TESTLIST_COMP" && $3->children.size() == 1){
            $3 = $3->children[0];
        }

        $$->children.push_back(node_1);

        $$->type = merge_power($1->type, $3->type);
        
    }
    ;


atom_expr
    : atom trailer_star {
        simplify_tree($1);
        $$ = create_node("ATOM_EXPR");
        $$->children.push_back($1);
        
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
       
        if($1->type_set){
            $$->type = $1->type;
            $$->type_set = true;
        }

        simplify_tree($1);
        
        free($2);

    }
    | atom DOT NAME trailer_star{
        simplify_tree($1);
        $$ = create_node("ATOM_EXPR");
        string new_name = $1->lexval + "." + $3;
        struct node_ast* node_1 = create_node_lex("NAME", new_name);
        
        
        node_1->is_dot = true;
        node_1->dot_name = $3;
        node_1->dot_obj = $1->lexval;
        // node_1->type = get_dot_type($1->lexval, $3);
        // node_1->type_set = true;
        node_1->entry = symbol_table_lookup($1->lexval);
        node_1->obj_class_name = node_1->entry->type;
        // cerr<<"obj_class_name: "<<node_1->obj_class_name<<endl;

        //get the type of the parameter
        string type = get_return_type_obj_variable(node_1->obj_class_name, $3);
        if(type == "NOTDEFINED"){
            //continue
        }else{
            $$->type = type;
            $$->type_set = true;
        }
        

        $$->children.push_back(node_1);

        // cerr<<"$1->lexval: "<<$1->lexval<<endl;
        for(int i = 0; i < $4->children.size(); i++){
            $$->children.push_back($4->children[i]);
        } 
        
        // table_entry* entry = symbol_table_lookup(new_name);      
        // if(entry != NULL){
        //     $$->type = entry->type;
        //     $$->type_set = true;
        // }

    }
    | atom LPAREN arglist RPAREN {
        $$ = create_node("FUNC_CALL");
        // cerr<<"$1->lexval: "<<$1->lexval<<endl;
        simplify_tree($1);
        $$->children.push_back($1);
        $$->children.push_back($3);
        
        if(!verify_formal_and_actual_arguements($1->lexval, $3->entries)){
            error_report("Syntax Error: Formal and Actual arguments do not match", yylineno);
        }
        else{
            // cerr<<"Formal and Actual arguments match"<<endl;
        }
        // cerr<<"$1->lexval: "<<$1->lexval<<endl;
        
        $$->type = get_return_type($1->lexval);
        // cerr<<"function name: "<<$1->lexval<<endl;

        $$->func_entry = func_table_entry($1->lexval);
        $$->type_set = true;
    }
    | atom DOT NAME LPAREN arglist RPAREN {
        $$ = create_node("FUNC_CALL");
        simplify_tree($1);

        $$->is_func_dot = true;
        $$->func_dot_name = $3;
        $$->func_dot_obj = $1->lexval;

        string new_name = $1->lexval + "." + $3;
        struct node_ast* node_1 = create_node_lex("NAME", new_name);
        $$->children.push_back(node_1);
        $$->children.push_back($5);

        


        if(!verify_formal_and_actual_arguements_dot($1->lexval, $3, $5->entries)){
            error_report("Syntax Error: Formal and Actual arguments do not match", yylineno);
        }
        else{
            // cerr<<"Formal and Actual arguments match 1"<<endl;
        }
        // cerr<<"new_name: "<<new_name<<endl;
        $$->type = get_return_type_dot($1->lexval, $3);
        $$->func_entry = func_table_entry_dot($1->lexval, $3);
        // cerr<<"function name: "<<$1->lexval<<endl;
        $$->type_set = true;
        
    }
    |atom DOT NAME LPAREN RPAREN {
        $$ = create_node("FUNC_CALL");
        simplify_tree($1);

        $$->is_func_dot = true;
        $$->func_dot_name = $3;
        $$->func_dot_obj = $1->lexval;

        string new_name = $1->lexval + "." + $3;
        struct node_ast* node_1 = create_node_lex("NAME", new_name);
        $$->children.push_back(node_1);
        struct node_ast* node_2 = create_node("EMPTY_PROD");
        $$->children.push_back(node_2);

        if(!verify_formal_and_actual_arguements_dot($1->lexval, $3, node_2->entries)){
            error_report("Syntax Error: Formal and Actual arguments do not match", yylineno);
        }
        else{
            // cerr<<"FormaTRAILERl and Actual arguments match 2"<<endl;
        }
        // cerr<<"function name: "<<$1->lexval<<endl;

        $$->type = get_return_type_dot($1->lexval, $3);
        $$->func_entry = func_table_entry_dot($1->lexval, $3);
        // cerr<<"function name: "<<$1->lexval<<endl;
        $$->type_set = true;
    }
    |atom LPAREN RPAREN {
        $$ = create_node("FUNC_CALL");
        simplify_tree($1);

        $$->children.push_back($1);
        struct node_ast* node_1 = create_node("EMPTY_PROD");
        $$->children.push_back(node_1);

        if(!verify_formal_and_actual_arguements($1->lexval, node_1->entries)){
            error_report("Syntax Error: Formal and Actual arguments do not match", yylineno);
        }
        else{
            // cerr<<"Formal and Actual arguments match 3"<<endl;
        }

        $$->type = get_return_type($1->lexval);
        $$->func_entry = func_table_entry($1->lexval);
        $$->type_set = true;

    }
    ;
    

trailer_star
    : {
        $$ = create_node("EMPTY_PROD");
    }
    | trailer_star trailer {
        $$ = create_node("TRAILER_STAR");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        free($1);
        $$->children.push_back($2);
    } 
    ;

atom
    : LPAREN RPAREN {
        $$ = create_node("EMPTY_PROD");
        $$->type = "NOTDEFINED";
        
    }
    | LPAREN testlist_comp RPAREN {
        $$ = create_node("ATOM");
        $$->children.push_back($2);
        simplify_tree($2);
        $2->arr_arg = false;

    }
    | LBRACKET RBRACKET {
        $$ = create_node("ATOM");
    }
    | LBRACKET error {
        error_report("Syntax Error: ']' expected for '[' at ", bracket.top());
    }
    | LBRACKET testlist_comp RBRACKET {
        $$ = create_node("ATOM");
        $$->children.push_back($2);
        simplify_tree($2);
        $2->arr_arg = true;
    }
    | LBRACKET testlist_comp error {
        error_report("Syntax Error: ']' expected for '[' at ", bracket.top());
    }
    | LBRACE RBRACE {
        $$ = create_node("ATOM");
    }
    | LBRACE error {
        error_report("Syntax Error: '}' expected for '{' at ", brace.top());
    }
    | NAME {
        $$ = create_node("ATOM");
        struct node_ast* node_1 = create_node_lex("NAME", $1);
        $$->children.push_back(node_1);
        table_entry* entry = symbol_table_lookup($1); 
        
        if(entry != NULL){ 
            
            if(entry->type=="class"){
                $$->type = $1;
                $$->type_set = true;
            }else{
                $$->type = entry->type;
                $$->type_set = true;
            
            }
            $$->entry = entry;
        }
    }
    | NUMBER {
        $$ = create_node("ATOM");
        struct node_ast* node_1 = create_node_lex("NUMBER", $1);

        $$->type = "NUMBER";
        $$->type_set = true;

        $$->children.push_back(node_1);
    }
    | extra_keywords {
        simplify_tree($1);
        $$ = create_node("ATOM");
        $$->children.push_back($1);
        if($1->type_set){
            $$->type = $1->type;
            $$->type_set = true;
        }
    }
    | NONE {
        $$ = create_node("ATOM");
        struct node_ast* node_1 = create_node_lex("NONE", "None");
        $$->children.push_back(node_1);
        $$->type = "NONE";
        $$->type_set = true;
    }
    | TRUE {
        $$ = create_node("ATOM");
        struct node_ast* node_1 = create_node_lex("BOOLVAL", "True");
        $$->children.push_back(node_1);
        $$->type = "BOOL";
        $$->type_set = true;
        
    }
    | FALSE {
        $$ = create_node("ATOM");
        struct node_ast* node_1 = create_node_lex("BOOLVAL", "False");
        $$->children.push_back(node_1);
        $$->type = "BOOL";
        $$->type_set = true;
    }
    ;



extra_keywords
    : STRING {
        //here a new lexval of the string is created
        $$ = create_node("EXTRA_KEYWORDS");
        string lexyval = $1;

        // std::string new_lexyval = replaceQuotes($1);
        string new_lexyval = $1;
        // cerr<<"new_lexyval: "<<new_lexyval<<endl;
        struct node_ast* node_1 = create_node_lex("STRING", new_lexyval);
        $$->children.push_back(node_1);
        $$->type = "STRING";
        $$->type_set = true;
        string label = create_new_string_label();
        label_to_string[label] = new_lexyval;
        string_to_label[new_lexyval] = label;

        

    }
    | INT{
        $$ = create_node("EXTRA_KEYWORDS");
        struct node_ast* node_1 = create_node("INT");
        
        $$->children.push_back(node_1);
        $$->type = "INT";
        $$->type_set = true;
    }
    | RANGE {
        $$ = create_node("EXTRA_KEYWORDS");
        struct node_ast* node_1 = create_node_lex("RANGE", "range");
        $$->children.push_back(node_1);
    }
    | PRINT {
        $$ = create_node("EXTRA_KEYWORDS");
        struct node_ast* node_1 = create_node_lex("PRINT", "print");
        $$->children.push_back(node_1);
        $$->type = "NOTDEFINED";
    }
    | LEN {
        $$ = create_node("EXTRA_KEYWORDS");
        struct node_ast* node_1 = create_node_lex("LEN", "len");
        $$->children.push_back(node_1);
        $$->type = "NOTDEFINED";
    }
    | BOOL {
        $$ = create_node("EXTRA_KEYWORDS");
        struct node_ast* node_1 = create_node("BOOL");
        $$->children.push_back(node_1);
        $$->type = "BOOL";
        $$->type_set = true;
    }
    | COMPLEX {
        $$ = create_node("EXTRA_KEYWORDS");
        struct node_ast* node_1 = create_node("COMPLEX");
        $$->children.push_back(node_1);
        $$->type = "COMPLEX";
        $$->type_set = true;
    }
    | STR {
        $$ = create_node("EXTRA_KEYWORDS");
        struct node_ast* node_1 = create_node("STR");
        $$->children.push_back(node_1);
        $$->type = "STR";
        $$->type_set = true;
    }
    | FLOAT {
        $$ = create_node("EXTRA_KEYWORDS");
        struct node_ast* node_1 = create_node("FLOAT");
        $$->children.push_back(node_1);
        $$->type = "FLOAT";
        $$->type_set = true;
    }
    | LIST {
        $$ = create_node("EXTRA_KEYWORDS");
        struct node_ast* node_1 = create_node("LIST");
        $$->children.push_back(node_1);
        $$->type = "LIST";
        $$->type_set = true;

    }
    ;

testlist_comp
    : test testlist_comp_line {
        $$ = create_node("TESTLIST_COMP");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }

        simplify_tree($1);

        if($1->type_set){
            $$->type = $1->type;
            $$->type_set = true;
        }

        free($2);
    }
    | test testlist_comp_line COMMA {
        $$ = create_node("TESTLIST_COMP");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        simplify_tree($1);
        
        free($2);
        // $$->children.push_back(create_node("COMMA"));
    }
    ;

testlist_comp_line
    : {
        $$ = create_node("EMPTY_PROD");
    }
    | testlist_comp_line COMMA test {
        $$ = create_node("TESTLIST_COMP_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        free($1);
        $$->children.push_back($3);
    }
    ;

trailer
    : LBRACKET subscriptlist RBRACKET {
        $$ = create_node("TRAILER");
        $$->children.push_back($2);
    }
    ;

subscriptlist
    : subscript subscriptlist_line {
        $$ = create_node("SUBSCRIPTLIST");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        free($2);
    }
    | subscript subscriptlist_line COMMA {
        $$ = create_node("SUBSCRIPTLIST");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        free($2);
    }
    ;

subscriptlist_line
    : {
        $$ = create_node("EMPTY_PROD");
    }
    | subscriptlist_line COMMA subscript {
        $$ = create_node("SUBSCRIPTLIST_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        free($1);
        $$->children.push_back(create_node("COMMA"));
        $$->children.push_back($3);
    }
    ;

subscript
    : test {
        $$ = create_node("SUBSCRIPT");
        $$->children.push_back($1);
    }
    | test COLON {
        $$ = create_node("SUBSCRIPT");
        $$->children.push_back($1);
    }
    | COLON test {
        $$ = create_node("SUBSCRIPT");
        $$->children.push_back($2);
    }
    | COLON sliceop {
        $$ = create_node("SUBSCRIPT");
        $$->children.push_back($2);
    }
    | test COLON test {
        $$ = create_node("SUBSCRIPT");
        $$->children.push_back($1);
        $$->children.push_back($3);
    }
    | COLON test sliceop {
        $$ = create_node("SUBSCRIPT");
        $$->children.push_back($2);
        $$->children.push_back($3);
    }
    | test COLON sliceop {
        $$ = create_node("SUBSCRIPT");
        $$->children.push_back($1);
        $$->children.push_back($3);
    }
    | test COLON test sliceop {
        $$ = create_node("SUBSCRIPT");
        $$->children.push_back($1);
        $$->children.push_back($3);
        $$->children.push_back($4);
    }
    ;

sliceop
    : COLON {
        $$ = create_node("SLICEOP");
    }
    | COLON test {
        $$ = create_node("SLICEOP");
        $$->children.push_back($2);
    }
    ;




testlist
    : test testlist_line {
        $$ = create_node("TESTLIST");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        free($2);
    }
    | test testlist_line COMMA {
        $$ = create_node("TESTLIST");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        free($2);
    }
    ;

testlist_line
    : {
        $$ = create_node("EMPTY_PROD");
    }
    | testlist_line COMMA test {
        $$ = create_node("TESTLIST_LINE");
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        free($1);
        $$->children.push_back($3);
    }
    ;

classdef
    : CLASS NAME COLON{
        $<node_ptr>$ = create_node("CLASSDEF");
        $<node_ptr>$->symbol_t = create_symbol_table_class($2, yylineno);
        allocate_parent_and_child($<node_ptr>$->symbol_t);

        CLASS_NODE_TEMP = $<node_ptr>$;

        table_stack.push($<node_ptr>$->symbol_t);

    } suite {
        $$ = CLASS_NODE_TEMP;
        struct node_ast* node_1 = create_node("CLASS");
        $$->children.push_back(node_1);
        $$->children.push_back(create_node_lex("NAME", $2));
        $$->children.push_back($5);


        // create a new type with the class name, calculate offset of all

        int size = calculate_class_type_size($$->symbol_t);
        type_size[$2] = 8;// since we will just use a pointer
        // add_entries_to_symbol_table($$->symbol_t, $5->entries);
    }

    | CLASS NAME LPAREN RPAREN COLON{
        $<node_ptr>$ = create_node("CLASSDEF");
        $<node_ptr>$->symbol_t = create_symbol_table_class($<str>2, yylineno);
        allocate_parent_and_child($<node_ptr>$->symbol_t);
        CLASS_NODE_TEMP = $<node_ptr>$;

        table_stack.push($<node_ptr>$->symbol_t);
    } suite {
        $$ = CLASS_NODE_TEMP;
        struct node_ast* node_1 = create_node("CLASS");
        $$->children.push_back(node_1);
        $$->children.push_back(create_node_lex("NAME", $2));
        $$->children.push_back($7);
        // add_entries_to_symbol_table($$->symbol_t, $7->entries);

        int size = calculate_class_type_size($$->symbol_t);
        type_size[$2] = 8;

    }
    | CLASS NAME LPAREN NAME RPAREN COLON {
        $<node_ptr>$ = create_node("CLASSDEF");
       
        $<node_ptr>$->symbol_t = create_symbol_table_class($<str>2, yylineno);
        allocate_parent_and_child($<node_ptr>$->symbol_t);
        CLASS_NODE_TEMP = $<node_ptr>$;
        table_stack.push($<node_ptr>$->symbol_t);
        //checks if the given class is inherited from a class that exists and adds this to it
        check_and_inherit_class($<str>4, $<node_ptr>$->symbol_t);
        //print entires in the symbol table

    }suite {
        
        $$ = create_node("CLASSDEF");
        $$ = CLASS_NODE_TEMP;
        struct node_ast* node_1 = create_node("CLASS");
        $$->children.push_back(node_1);
        $$->children.push_back(create_node_lex("NAME", $2));
        $$->children.push_back(create_node_lex("NAME", $4));
        $$->children.push_back($8);
        // add_entries_to_symbol_table($$->symbol_t, $8->entries);
        int size = calculate_class_type_size($$->symbol_t);
        type_size[$2] = 8;

        

    }
    ;

arglist
    : argument arglist_line {
        $$ = create_node("ARGLIST");
        $$->children.push_back($1);
        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        $$->entries.push_back($1->entry);
       
        for(int i = 0; i < $2->entries.size(); i++){
            $$->entries.push_back($2->entries[i]);
        }

        free($2);
    }
    | argument arglist_line COMMA {
        $$ = create_node("ARGLIST");
        $$->children.push_back($1);
        $$->entries = $2->entries;
        add_entry_to_entries($$->entries, $1->entry);

        for(int i = 0; i < $2->children.size(); i++){
            $$->children.push_back($2->children[i]);
        }
        free($2);
    }
    ;

arglist_line
    : {
        $$ = create_node("EMPTY_PROD");
        $$->entries = vector<table_entry*>();
        
    }
    | arglist_line COMMA argument{
        $$ = create_node("ARGLIST_LINE");
        
        for(int i = 0; i < $1->children.size(); i++){
            $$->children.push_back($1->children[i]);
        }
        $$->children.push_back($3);
        $$->entries.push_back($3->entry);
        for(int i = 0; i < $1->entries.size(); i++){
            $$->entries.push_back($1->entries[i]);
        }
        free($1);

    }
    ;

argument
    : test {
        $$ = create_node("ARGUMENT");
        //create an entry out of the arguement
        simplify_tree($1);
        // cerr<<"$1->name: "<<$1->name<<endl;
        $$->children.push_back($1);


        if($1->name == "NAME"){
           if($1->is_dot){
                //continue
                // cerr<<"hello"<<$1->dot_obj<<endl;
                // $1->type = get_dot_type($1->dot_obj, $1->dot_name);
                // $1->type_set = true;
            }
            else if(symbol_table_lookup($1->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }
        else if($1->name == "ATOM_EXPR"){
            if($1->children.size() == 2){
                $1->type = get_array_ref_type($1->children[0]->lexval, $1->children[1]);
                // cerr<<"$1->type: "<<$1->type<<endl;
                $1->array_entry = symbol_table_lookup($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval+"["+$1->children[1]->lexval+"]";
            }
        }else if($1->name == "FUNC_CALL"){
            if($1->is_func_dot){
                $1->type = get_return_type_dot($1->func_dot_obj, $1->func_dot_name);
                $1->type_set = true;
                $1->lexval = $1->func_dot_obj + "." + $1->func_dot_name;
            }else{
                $1->type = get_return_type($1->children[0]->lexval);
                $1->type_set = true;
                $1->lexval = $1->children[0]->lexval;
            }
        }else if($1->name == "TESTLIST_COMP" && $1->children.size() == 1){
            $1 = $1->children[0];
        }
        
        
        $$->entry = create_table_entry($1->lexval, $1->type, NULL, yylineno, "argument", offset);
        // cerr<<"hello"<<endl;
        if($1->type_set){
            $$->type = $1->type;
            $$->type_set = true;
        }
        
    }
    | atom_expr EQUAL atom_expr {
        $$ = create_node("ARGUMENT");
        $$->children.push_back($1);
        $$->children.push_back(create_node("EQUAL"));
        $$->children.push_back($3);

        simplify_tree($1);
        simplify_tree($3);
        if($3->name == "NAME"){
            if(symbol_table_lookup($3->lexval) == NULL){
                error_report("Syntax Error: Variable not defined", yylineno);
            }
        }

        $$->entry = create_table_entry($1->lexval, $1->type, NULL, yylineno, "argument", offset);
        $$->type = $1->type;
        $$->type_set = true;


    }
    ;

%%


int main(int argc, char *argv[]) {

   for (int i = 1; i < argc; i++) {
       std::string arg = argv[i];
       if (arg == "-v" || arg == "--verbose") {
           verbose = 1;
       } else if (arg == "--help" || arg == "-h") {
           std::cout << "Usage: [program_name] [options]\n"
                     << "Options:\n"
                     << "  -v, --verbose   Enable verbose output\n"
                     << "  --input       <file>   Specify input python file path\n"
                     << "  --output      <file>   Specify output file for the dot script\n"
                     << "  --graph         <png or pdf> Specify the type of graph\n"
                     << "  --help         Display this help message\n";
           return 0;
       } else if (arg == "--input") {
           if (i + 1 < argc) {
               input_file = argv[i + 1];
               yyin = fopen(input_file, "r");
                if (!yyin) {
                    perror(argv[1]);
                    return 1;
                }
               i++;
           } else {
               fprintf(stderr, "Error: --input option requires one argument.\n");
               return 1;
           }
       } else if (arg == "--output") {
           if (i + 1 < argc) {
               dotfile_path = argv[i + 1];
               i++;
           } else {
               fprintf(stderr, "Error: -output option requires one argument.\n");
               return 1;
           }
       } else if(arg == "--graph"){
           if (i + 1 < argc) {
                ast_type = argv[i+1];
               i++;
           } else {
               fprintf(stderr, "Error: -graph option requires one argument.\n");
               return 1;
           }
       } else {
           std::cout << "Unknown option: " << arg << std::endl;
           std::cout << "use --help or -h to get some details of use." << arg << std::endl;
           exit(1);
       }
   }

    table_stack.push(global_table);
    add_common_funcs();

    return yyparse();

    if(yyin){
        fclose(yyin);

    }
}

int yywrap() {
    return 1;
}
