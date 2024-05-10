# CS335_Project

A python compiler made as a course project for the course CS 335 -  Compiler Design (2023-2034). Made by a team of 3 Pratham Sahu, Aditya Bangar and Abir Rajbongshi

## Running

We have used flex+ to implement the lexer and bison for the parser. Both have been written in c++.

And as asked for in the problem statement, we have used DOT to generate the AST


We have created a script file run.sh to automate the compilation and testing of the lexer and the parser

The following options are provided in the execution of our code

- `--input` - Use this option to specify the input .py file.  

- `--output` - Use this option to specify the output dot file.

- `--help` - Use this option to display help message.

- `--verbose` -  Use this to display verbose output, on the terminal.

- `--graph` - To specify the output type for the AST, it will be of the same name as the output file. It can either be `png` or `pdf`

run make to create the binary
> `make`  

To run the binary, use the following command along with command line arguments:
> `./pycomp`

To clean the binary, use the following command:
> `make clean`



The AST is created as a png or a pdf file. The 3ac and the symbol table are stored in .tac and .csv files respectively. The assembly code is stored in a x86_code.s file.

To generate the x86 code for a given py file, use the following command:

```
./run_x86 <testcase>
```

This python compiler supports the following functionalities:

- Arithmetic operations
- Logical operations
- Conditional statements
- Loops, Control statements
- Functions
- Recursion
- Method and Static method calls
- Classes, Multilevel Inheritance
- Exception Handling
- Lists of primitive and non-primitive types

This compiler is tested on a number of test cases and is able to generate the correct output for the same. 
The `tests` folder contains the test cases that we have used to test our compiler.

The experience in building this compiler has been very enriching and we have learnt a lot about the working of a compiler and the various stages involved in the same. However rudimentary the compiler may be, it supports a lot of functionalities and we are proud of the work we have done.
