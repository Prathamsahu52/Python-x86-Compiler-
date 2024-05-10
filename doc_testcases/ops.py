class MathOperations:
    
    def __init__(self, x: int):
        self.x:int = x
    
    def add(x: int, y: int) -> int:
        return x + y
    
    def subtract(x: int, y: int) -> int:
        return x - y
    
    def multiply(x: int, y: int) -> int:
        return x * y
    
    def power(x:int, y:int)->int:
        i:int=0
        j:int=y-1
        for i in range(j):
            # print(x)
            x=x*x
        return x
    
    def floordiv(x:int,y:int)->int:
        return x // y
    
    def bitAnd(x:int, y:int)->int:
        return x & y
    
    def bitOr(x:int, y:int)->int:
        return x | y
    
    def bitXor(x:int, y:int)->int:
        return x ^ y

def main() -> None:
    x: int = 5
    y: int = 3
    
    # Using static methods
    result_add: int = MathOperations.add(x, y)
    result_subtract: int = MathOperations.subtract(MathOperations.add(x,y), y)
    result_multiply: int = MathOperations.multiply( MathOperations.subtract(MathOperations.add(x,y), y), y)
    result_pow:int=MathOperations.power(x,y)
    result_floordiv:int=MathOperations.floordiv(x,y)
    result_and:int=MathOperations.bitAnd(x,y)
    result_or:int=MathOperations.bitOr(x,y)
    result_xor:int=MathOperations.bitXor(x,y)
    # Using instance method
    math_ops: MathOperations = MathOperations(x)
    
    
    print( result_add)
    print(result_subtract)
    print(result_multiply)
    print(result_pow)
    print(result_floordiv)
    print(result_and)
    print(result_or)
    print(result_xor)
    print(x)

if __name__ == "__main__":
    main()