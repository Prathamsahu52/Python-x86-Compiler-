## Using this testcase, we show the following features:

##1. How multilevel inheritence works in our compiler

##2. We can define lists of objects of a class and iterate over them,
##  call their functions and access their variables. Lists are not
## limited to primitive types.

class Animal:
    def __init__(self, name: str) -> None:
        self.name: str = name

    def speak(self) -> None:
        i:int=0


class Dog(Animal):
    def __init__(self, name: str, breed: str) -> None:
        # The variables name is inherited from the parent class
        self.name = name
        self.breed: str = breed
    def speak(self) -> None:
        print(self.name)
        print("says woof")
        print("====")


class Labrador(Dog):
    def __init__(self, name: str, breed: str, color: str) -> None:
        self.name = name
        self.breed = breed
        self.color: str = color
    
    def display_info(self) -> None:
        print(self.name)
        print("is a")
        print(self.color)
        print(self.breed)
        print("====")
    def speak(self) -> None:
        print(self.name)
        print("says woof")
        print("====")

# Example usagein

def main() -> None:
    labrador1:Labrador = Labrador("Max", "Labrador Retriever", "Golden")
    labrador2:Labrador = Labrador("Buddy", "Labrador Retriever", "Black")
    labrador3:Labrador = Labrador("Charlie", "Labrador Retriever", "Brown")

    # We support list of objects
    labrador_list: list[Labrador] = [labrador1, labrador2, labrador3]
    
    labrador: int = 0
    for labrador in range(len(labrador_list)):
        k: Labrador = labrador_list[labrador]
        k.display_info()
        k.speak()

    # the parent class functions can be called using its objects.

    dog1: Dog = Dog("Spike", "German Shepherd")
    dog2: Dog = Dog("Bella", "Golden Retriever")
    dog3: Dog = Dog("Rusty", "Poodle")


    dog_list: list[Dog] = [dog1, dog2, dog3]
    dog: int = 0
    for dog in range(len(dog_list)):
        d: Dog = dog_list[dog]
        d.speak()

if __name__ == "__main__":
    main()