%option noyywrap
%option yylineno

%{
    #include <unordered_map>
    #include <string>   
    #include <iostream>
    #include <set>
    #include <algorithm>
    #include <stack>
    #include <cstdio>

    using namespace std;
    int g_current_line_indent = 0;

    stack<int> indentStack;
    static const unsigned int TAB_WIDTH = 2;
    #define YY_USER_INIT { \
        indentStack.push(0); \
        BEGIN(initial); \
    }

    extern int verbose;
    int implicit_open = 0;

    stack<int > paren;
    stack<int> brace;
    stack<int> bracket;
    
    #include "parser.tab.h"  // Include Bison-generated parser header

    int getLineNo(int yylineno, string yytext){
        int lineNumber = yylineno;
        for(int i = 0; i < yytext.length(); i++){
            if(yytext[i] == '\n'){
                lineNumber--;
            }
        }

        return lineNumber;
    }

%}

%x initial
%x indent
%x explicit_state
%s normal


KEYWORDS    "False"|"None"|"True"|"as"|"assert"|"async"|"await"|"break"|"class"|"continue"|"def"|"del"|"elif"|"else"|"except"|"finally"|"for"|"from"|"global"|"if"|"import"|"in"|"is"|"lambda"|"nonlocal"|"pass"|"raise"|"return"|"try"|"while"|"with"|"yield"|"list"|"len"
LOGICAL_OP      "and"|"or"|"not"

DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          ({UNDERSCORE}|{LETTER})(({UNDERSCORE}|{LETTER}|{DIGIT}))*

UNDERSCORE  "_"
DECINTEGER  ([1-9]({UNDERSCORE}?{DIGIT})*)|("0"+({UNDERSCORE}"0")*)
BININTEGER  "0"("B"|"b")({UNDERSCORE}?("0"|"1"))+
OCTINTEGER  "0"("O"|"o")({UNDERSCORE}?[0-7])+
HEXINTEGER  "0"("X"|"x")({UNDERSCORE}?({DIGIT}|[a-fA-F]))+
INTEGER_LITERAL     {DECINTEGER}|{HEXINTEGER}|{OCTINTEGER}|{BININTEGER}


DIGIT_PART  {DIGIT}({UNDERSCORE}?{DIGIT})*
FRACTION    "."{DIGIT_PART}
POINT_FLOAT      ({DIGIT_PART}?{FRACTION})|({DIGIT_PART}".")
EXPONENT    [eE][+-]?{DIGIT_PART}
EXPONENT_FLOAT      ({POINT_FLOAT}|{DIGIT_PART}){EXPONENT}
FLOATING_LITERAL    {POINT_FLOAT}|{EXPONENT_FLOAT}

IMAGINARY_LITERAL   ({FLOATING_LITERAL}|{DIGIT_PART})("j"|"J")

DELIMITER   "("|")"|"["|"]"|"{"|"}"|","|":"|"."|";"|"@"|"->"|"//="

STRING_PREFIX   "r"|"u"|"R"|"U"|"f"|"F"|"fr"|"Fr"|"fR"|"FR"|"rf"|"rF"|"Rf"|"RF"
SHORT_STRING_CHAR   [^\n\'\"\\]
ESCAPE_SEQUENCE \\.
SHORT_STRING_ITEM   {SHORT_STRING_CHAR}|{ESCAPE_SEQUENCE}
SHORT_STRING    (\'{SHORT_STRING_ITEM}*\')|(\"{SHORT_STRING_ITEM}*\")
LONG_STRING_CHAR    [^\\]
LONG_STRING_ITEM    {ESCAPE_SEQUENCE}|{LONG_STRING_CHAR}
LONG_STRING     (\'\'\'{LONG_STRING_ITEM}*\'\'\')|(\"\"\"{LONG_STRING_ITEM}*\"\"\")
MULTILINE_COMMENTS \n{WS}*{LONG_STRING}
STRING_LITERAL  ({STRING_PREFIX}?)({SHORT_STRING}|{LONG_STRING})

WS          [ \t\r\n]
COMMENTS    "#"[^\n]*
ARITHEMATIC_OP  "+"|"-"|"*"|"/"|"//"|"%"|"**"
RELATIONAL_OP   "=="|"!="|">"|"<"|">="|"<="

BITWISE_OP      "&"|"\|"|"\^"|"~"|"<<"|">>"
ASSIGNMENT_OP   "="|"+="|"-="|"*="|"/="|"%="|"**="|"&="|"|="|"^="|"<<="|">>="

INVALID_CHAR    "$"|"?"

%%


<initial>. {
    unput(yytext[0]);
    BEGIN(indent);
}
<initial>\n {
    if (verbose == 1) printf("initial mode\n");
    BEGIN(indent);
}
<indent>\t {
    if (verbose == 1) printf("indent mode: TAB\n");
    g_current_line_indent += TAB_WIDTH;
}
<indent>\n {
    if (verbose == 1) printf("ignoring empty line\n");
    g_current_line_indent = 0;
    // return NEWLINE;
}
<indent>" " {
    if (verbose == 1) printf("indent mode: SPACE\n");
    g_current_line_indent++;
}

<indent>. {
    unput(yytext[0]);
    if (verbose == 1) printf("indent: %d\n", g_current_line_indent);
    if (g_current_line_indent > indentStack.top()) {
        
        if (verbose == 1) printf("indent greater than top of stack: %d\n", g_current_line_indent);
        indentStack.push(g_current_line_indent);
        BEGIN(normal);   
        // cout << "sent token_indent\n";
        return TOKEN_INDENT;
    } else if (g_current_line_indent < indentStack.top()) {
        // return TOKEN_DEDENT for each level of indentation that is smaller until the indentation level is equal to the current indentation level
        //if it does not exist, return error

        while (g_current_line_indent < indentStack.top()) {
            if (verbose == 1) printf("dedent: %d\n", indentStack.top());
            indentStack.pop();
            // cout << "sent token_dedent\n";
            return TOKEN_DEDENT;
        }
        
        if(g_current_line_indent != indentStack.top()){
            if (verbose == 1) printf("Error: Indentation Error\n");
            return TOKEN_ERROR;
        }
        BEGIN(normal);

    }else{
        // cout << "yytext is : " << yytext[0] << endl;
        // unput(yytext[0]);
        if (verbose == 1) printf("Indent same as top of stack\n");
        BEGIN(normal);
    }
}

<indent><<EOF>> {
    
    // When encountering the end of file, we want to emit an
    // outdent for all indents currently left.
    while(indentStack.size() > 1){
        if (verbose == 1) printf("dedent: %d\n", indentStack.top());
        indentStack.pop();
        // cout << "sent token_dedent\n";
        return TOKEN_DEDENT;
    }
    // if(indentStack.size()==1){
    //     yyterminate();
    // }
    if (verbose == 1) printf("EOF\n");
    return ENDMARKER;
}

<normal>\n {
    if(implicit_open >= 1){
        // do nothing
    } else {
        if (verbose == 1) printf("normal ended, starting indent mode on line: %d\n", yylineno);
        g_current_line_indent = 0;
        BEGIN(indent);
        return NEWLINE;
    }
}

<normal><<EOF>> {
    if(implicit_open >= 1){
        int top_paren = paren.size() > 0 ? paren.top() : 0;
        int top_brace = brace.size() > 0 ? brace.top() : 0;
        int top_bracket = bracket.size() > 0 ? bracket.top() : 0;

        int line_max= max(top_paren, max(top_brace, top_bracket));
        cerr << "Syntax Error, line : " << getLineNo(line_max, yytext) << " Mismatched paren/bracket/brace" << endl;
        exit(1);
    }
    unput('\n');
}
<normal>"int"       { 
    if (verbose == 1) printf("INT\n");
    return INT; 
}
<normal>"float"     { 
    if (verbose == 1) printf("FLOAT\n");
    return FLOAT; 
}
<normal>"str"       { 
    if (verbose == 1) printf("STR\n");
    return STR; 
}
<normal>"bool"      { 
    if (verbose == 1) printf("BOOL\n");
    return BOOL; 
}
<normal>"complex"   { 
    if (verbose == 1) printf("COMPLEX\n");
    return COMPLEX; 
}
<normal>"print"     { 
    if (verbose == 1) printf("PRINT\n");
    return PRINT; 
}
<normal>"range"     { 
    if (verbose == 1) printf("RANGE\n");
    return RANGE; 
}

<normal>"len"       { 
    if (verbose == 1) printf("LEN\n");
    return LEN; 
}
<normal>"\\" {
    if (verbose == 1) printf("explicit line joiner detected\n");
    BEGIN(explicit_state);
}

<explicit_state>"\n" {
    if (verbose == 1) printf("explicit_state : ignoring the newline character\n");
    BEGIN(normal);
}

<normal>{KEYWORDS}  { 
    // cerr<<"KEYWORD: "<<yytext<<endl;
    if (verbose == 1) printf("KEYWORD: %s\n", yytext);
    yylval.str = strdup(yytext);

    
    if (strcmp(yytext, "False") == 0) return FALSE;
    else if (strcmp(yytext, "None") == 0) return NONE;
    else if (strcmp(yytext, "True") == 0) return TRUE;
    else if (strcmp(yytext, "as") == 0) return AS;
    else if (strcmp(yytext, "assert") == 0) return ASSERT;
    else if (strcmp(yytext, "async") == 0) return ASYNC;
    else if (strcmp(yytext, "await") == 0) return AWAIT;
    else if (strcmp(yytext, "break") == 0) return BREAK;
    else if (strcmp(yytext, "class") == 0) return CLASS;
    else if (strcmp(yytext, "continue") == 0) return CONTINUE;
    else if (strcmp(yytext, "def") == 0) return DEF;
    else if (strcmp(yytext, "del") == 0) return DEL;
    else if (strcmp(yytext, "elif") == 0) return ELIF;
    else if (strcmp(yytext, "else") == 0) return ELSE;
    else if (strcmp(yytext, "except") == 0) return EXCEPT;
    else if (strcmp(yytext, "finally") == 0) return FINALLY;
    else if (strcmp(yytext, "for") == 0) return FOR;
    else if (strcmp(yytext, "from") == 0) return FROM;
    else if (strcmp(yytext, "global") == 0) return GLOBAL;
    else if (strcmp(yytext, "if") == 0) return IF;
    else if (strcmp(yytext, "import") == 0) return IMPORT;
    else if (strcmp(yytext, "in") == 0) return IN;
    else if (strcmp(yytext, "is") == 0) return IS;
    else if (strcmp(yytext, "lambda") == 0) return LAMBDA;
    else if (strcmp(yytext, "nonlocal") == 0) return NONLOCAL;
    else if (strcmp(yytext, "pass") == 0) return PASS;
    else if (strcmp(yytext, "raise") == 0) return RAISE;
    else if (strcmp(yytext, "return") == 0) return RETURN;
    else if (strcmp(yytext, "try") == 0) return TRY;
    else if (strcmp(yytext, "while") == 0) return WHILE;
    else if (strcmp(yytext, "with") == 0) return WITH;
    else if (strcmp(yytext, "yield") == 0) return YIELD;
    else if (strcmp(yytext, "list") == 0) return LIST;
}


<normal>{INTEGER_LITERAL}   { 
    if (verbose == 1) printf("INTEGER_LITERAL: %s\n", yytext);
    yylval.str = strdup(yytext);
    return NUMBER; 
}

<normal>{FLOATING_LITERAL}  { 
    if (verbose == 1) printf("FLOATING_LITERAL: %s\n", yytext);
    yylval.str = strdup(yytext);
    return NUMBER; 
}

<normal>{IMAGINARY_LITERAL} { 
    if (verbose == 1) printf("IMAGINARY_LITERAL: %s\n", yytext);
    yylval.str = strdup(yytext);
    return NUMBER; 
}

<normal>{STRING_LITERAL}    { 
    if (verbose == 1) printf("STRING_LITERAL: %s\n", yytext);
    yylval.str = strdup(yytext);
    
    return STRING;
}


<normal>{DELIMITER} { 
    if (verbose == 1) printf("DELIMITER: %s\n", yytext);
    yylval.str = strdup(yytext);
    if (yytext[0] == '(') {
        
        paren.push(getLineNo(yylineno, yytext));
        if(verbose == 1) cout << "LPAREN: starting implicit_open \n";
        implicit_open += 1;
        return LPAREN;
    }
    else if (yytext[0] == ')') {

        if(paren.size() > 0){
            if(verbose == 1) cout<<"closing open at line: "<<paren.top()<<endl;
            paren.pop();
        }else{
            cerr << "Syntax Error, line : " << yylineno << " Mismatched paren " << endl;
            exit(1);
        }
        if(verbose == 1) cout << "RPAREN: closing implicit_open \n";
        if(implicit_open > 0){
            implicit_open -= 1;
        }
        return RPAREN;
    }
    else if (yytext[0] == '[') {
        bracket.push(getLineNo(yylineno, yytext));
        if(verbose == 1) cout << "LBRACKET: starting implicit_open \n";
        implicit_open += 1;
        return LBRACKET;
    }
    else if (yytext[0] == ']') {
        if(bracket.size() > 0){
            if(verbose == 1) cout<<"closing open at line: "<<bracket.top()<<endl;
            bracket.pop();
        }else{
            cerr << "Syntax Error, line : " << yylineno << " Mismatched bracket" << endl;
            exit(1);
        }
        if(verbose == 1) cout << "RBRACKET: closing implicit_open \n";
        if(implicit_open > 0){
            implicit_open -= 1;
        }
        return RBRACKET;
    }
    else if (yytext[0] == '{'){
        brace.push(getLineNo(yylineno, yytext));
        if(verbose == 1) cout << "LBRACE: starting implicit_open \n";


        return LBRACE;
    }
    else if (yytext[0] == '}') {

        if(brace.size() > 0){
            if(verbose == 1) cout<<"closing open at line: "<<brace.top()<<endl;
            brace.pop();
        }else{
            cerr << "Syntax Error, line : " << yylineno << " Mismatched brace" << endl;
            exit(1);
        }
        return RBRACE;
    }
    else if (yytext[0] == ',') return COMMA;
    else if (yytext[0] == ':') return COLON;
    else if (yytext[0] == '.') return DOT;
    else if (yytext[0] == ';') return SEMICOLON;
    else if (yytext[0] == '@') return AT;
    else if (strcmp(yytext, "->") == 0) return ARROW;
    else if (strcmp(yytext, "//=") == 0) return FLOORDIV_ASSIGN;
}

<normal>{ARITHEMATIC_OP}    { 
    if (verbose == 1) printf("ARITHEMATIC_OP: %s\n", yytext);
    yylval.str = strdup(yytext);
    if (yytext[0] == '+') return PLUS;
    else if (yytext[0] == '-') return MINUS;
    else if (strcmp(yytext, "**") == 0) return DOUBLESTAR;
    else if (strcmp(yytext, "//") == 0) return DOUBLESLASH;
    else if (yytext[0] == '*') return STAR;
    else if (yytext[0] == '/') return SLASH;
    else if (yytext[0] == '%') return PERCENT;
    
}

<normal>{RELATIONAL_OP} { 
    if (verbose == 1) printf("RELATIONAL_OP: %s\n", yytext);
    yylval.str = strdup(yytext);
    if (strcmp(yytext, "==") == 0) return EQ;
    else if (strcmp(yytext, "!=") == 0) return NE;
    else if (strcmp(yytext, ">=") == 0) return GE;
    else if (strcmp(yytext,">") == 0) return GT;
    else if (strcmp(yytext, "<=") == 0) return LE;
    else if (strcmp(yytext, "<")==0) return LT;
    
   
}


<normal>{LOGICAL_OP}    { 
    if (verbose == 1) printf("LOGICAL_OP: %s\n", yytext);
    yylval.str = strdup(yytext);
    if (strcmp(yytext, "and") == 0) return AND;
    else if (strcmp(yytext, "or") == 0) return OR;
    else if (strcmp(yytext, "not") == 0) return NOT;
}
<normal>{ID}        { 
    if (verbose == 1) printf("ID: %s\n", yytext);
    // cout << "NAME : '" << yytext << "'" << endl;
    yylval.str = strdup(yytext);
    return NAME; 
}

<normal>{BITWISE_OP}    { 
    if (verbose == 1) printf("BITWISE_OP: %s\n", yytext);
    yylval.str = strdup(yytext);
    if (yytext[0] == '&') return AMPER;
    else if (yytext[0] == '|') return PIPE;
    else if (yytext[0] == '^') return CARET;
    else if (yytext[0] == '~') return TILDE;
    else if (strcmp(yytext, "<<") == 0) return LSHIFT;
    else if (strcmp(yytext, ">>") == 0) return RSHIFT;
}

<normal>{ASSIGNMENT_OP} { 
    if (verbose == 1) printf("ASSIGNMENT_OP: %s\n", yytext);
    yylval.str = strdup(yytext);
    if (yytext[0] == '=') return EQUAL;
    else if (strcmp(yytext, "+=") == 0) return PLUSEQUAL;
    else if (strcmp(yytext, "-=") == 0) return MINUSEQUAL;
    else if (strcmp(yytext, "*=") == 0) return STAREQUAL;
    else if (strcmp(yytext, "/=") == 0) return SLASHEQUAL;
    else if (strcmp(yytext, "%=") == 0) return PERCENTEQUAL;
    else if (strcmp(yytext, "**=") == 0) return DOUBLESTAREQUAL;
    else if (strcmp(yytext, "&=") == 0) return AMPEREQUAL;
    else if (strcmp(yytext, "|=") == 0) return PIPEEQUAL;
    else if (strcmp(yytext, "^=") == 0) return CARETEQUAL;
    else if (strcmp(yytext, "<<=") == 0) return LSHIFTEQUAL;
    else if (strcmp(yytext, ">>=") == 0) return RSHIFTEQUAL;
}

<normal>{INVALID_CHAR}  { 
    if (verbose == 1) printf("INVALID_CHAR: %s\n", yytext);
    // cerr << "Invalid character: " << yytext << endl;
}

<indent>{COMMENTS}  { /* Ignore comments */ if (verbose == 1) cout << "inside comments" << yytext << endl;}
<normal>{COMMENTS}  { /* Ignore comments */ if (verbose == 1) cout << "inside comments" << yytext << endl;}
<normal>{MULTILINE_COMMENTS} {  if (verbose == 1) cout << "multiline comment found \n";}

<normal>{WS}        { /* Ignore whitespace */}

<normal>.           { /* Handle invalid tokens */ 
                        if(verbose == 1) cerr << "Syntax Error, line : " << yylineno << " Possible wrong string" << endl;
                        if(verbose == 0) cerr << "Error: syntax error" << endl;
                        exit(1);
                    }


%%

// Additional C code, if needed, such as the main function

// int main(int argc, char **argv) {
//     // Call the lexer
//     yylex();
//     return 0;
// }
