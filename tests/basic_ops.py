##In this test case we show the following features:

##1. How to define and use static methods of a class. Functions without
##   self parameter are static methods.

##2. How to define and use instance methods of a class. Functions with
##   self parameter are instance methods.

##3. We show the functioning of basic arithmetic, bitwise operations

##4. We show how to pass function results as arguments to other functions
##  with infinite chaining
class MathOperations:
    
    def __init__(self, x: int):
        self.x:int = x
    
    def add(x: int, y: int) -> int:
        return x + y
    
    def subtract(x: int, y: int) -> int:
        return x - y
    
    def multiply(x: int, y: int) -> int:
        return x * y
    
    def divide(x: int, y: int) -> int:
        return x / y
    
    def power(x: int, y: int) -> int:
        return x ** y
    
    
    def bitAnd(x:int, y:int)->int:
        return x & y
    
    def bitOr(x:int, y:int)->int:
        return x | y
    
    def bitXor(x:int, y:int)->int:
        return x ^ y
    
    def leftShift(x:int, y:int)->int:
        return x << y
    
    def rightShift(x:int, y:int)->int:
        return x >> y

def main() -> None:
    x: int = 5
    y: int = 3
    
    # Using static methods
    result_add: int = MathOperations.add(x, y)

    ## we can pass functions results as arguments to other functions wiht infinite chaining

    result_subtract: int = MathOperations.subtract(MathOperations.add(x,y), y)
    result_multiply: int = MathOperations.multiply( MathOperations.subtract(MathOperations.add(x,y), y), y)
    result_power: int = MathOperations.power(x,y)
    result_and:int=MathOperations.bitAnd(x,y)
    result_or:int=MathOperations.bitOr(x,y)
    result_xor:int=MathOperations.bitXor(x,y)
    
    result_left_shift:int=MathOperations.leftShift(x,y)
    result_right_shift:int=MathOperations.rightShift(x,y)
    # Using instance method
    math_ops: MathOperations = MathOperations(x)
    
    
    print(result_add)
    print(result_subtract)
    print(result_multiply)
    print(result_power)
    print(result_and)
    print(result_or)
    print(result_xor)
    print(result_left_shift)
    print(result_right_shift)
    print(x)

if __name__ == "__main__":
    main()

