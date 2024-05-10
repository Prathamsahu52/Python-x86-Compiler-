CC=g++ -std=c++11
CFLAGS=-x c++

LEXER_SRC=src/lexer.l
PARSER_SRC=src/parser.y
LEXER_OUTPUT=lex.yy.c
PARSER_OUTPUT=parser.tab.c
PARSER_HEADER=parser.tab.h
SYMBOL_TABLE_SRC=src/symbol_table.cpp
TRYOUT_OUTPUT=pycomp
CSV_OUTPUT=csv_output.csv
3AC_OUTPUT=3ac_output.txt



all: flex_compile bison_compile executable_compile 


flex_compile:
	flex $(LEXER_SRC)

bison_compile:
	bison -d $(PARSER_SRC)



executable_compile:
	@$(CC) $(CFLAGS) -o $(TRYOUT_OUTPUT) $(PARSER_OUTPUT) $(LEXER_OUTPUT) -ll; \
	if [ $$? -eq 0 ]; then \
		echo "SUCCESS : $(TRYOUT_OUTPUT) created."; \
	else \
		echo "COMPILATION FAILED!"; \
	fi

clean:
	rm -f $(LEXER_OUTPUT) $(PARSER_OUTPUT) $(PARSER_HEADER) $(TRYOUT_OUTPUT) $(CSV_OUTPUT) $(3AC_OUTPUT) a.out
