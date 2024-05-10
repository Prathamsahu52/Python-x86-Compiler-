## In this test case we show the following features:

##1 We show the use of multiple for loops, while loops, if else statements, continue and break

def main():
    # use multiple for loops, while loops, if else statements, continue and break
    i: int = 0
    
    for i in range(5):
        print("In outer loop")
        while(i < 3):
            print("In inner loop")
            i += 1
            print(i)
            if i == 2:
                wall: str = "wall"
                break
            else:
                wall: str = "no wall"
                continue
    
    if i == 1:
        print("i is 1")
    elif i == 2:
        print("i is 2")
    elif i == 3:
        print("i is 3")
    elif i == 4:
        print("i is 4")
    elif i == 5:
        print("i is 5")
    else:
        print("i is not in the range 1-5")


if __name__ == "__main__":
    main()
