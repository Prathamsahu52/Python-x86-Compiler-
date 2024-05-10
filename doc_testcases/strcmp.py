def string_comparison(string1: str, string2: str,n1:int, n2:int) -> str:
    
    if n1==0 or n2==0 :
        return "One or both strings are empty."
    elif string1 == string2:
        return "The strings are equal."
    elif string1 < string2:
        return "The first string is lexicographically smaller than the second string."
    else:
        return "The first string is lexicographically greater than the second string."

def main():

    s1: str = "apple"
    s2: str = "banana"
    result:str=string_comparison(s1, s2,5,6)
    print(result)  

    s3: str = "world"
    s4: str = "hello"

    result= string_comparison(s3, s4, 6, 5)
    print(result)  

    s5: str = "python"
    s6: str = "python"
    result = string_comparison(s5, s6,6,6)
    print(result)  

if __name__ == "__main__":
    main()