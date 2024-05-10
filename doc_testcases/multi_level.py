class Animal:
    def __init__(self, name: str) -> None:
        self.name: str = name

    def speak(self) -> None:
        i:int=0


class Dog(Animal):
    def __init__(self, name: str, breed: str) -> None:
        self.name: str = name
        self.breed: str = breed

    def speak(self) -> None:
        print("woof")


class Labrador(Dog):
    def __init__(self, name: str, breed: str, color: str) -> None:
        self.name: str = name
        self.breed: str = breed
        self.color: str = color

    def display_info(self) -> None:
        print(self.name)
        print(self.color)
        print(self.breed)

# Example usagein

def main() -> None:
    labrador:Labrador = Labrador("Max", "Labrador Retriever", "Golden")
    labrador.display_info()  # Output: Max is a Golden Labrador Retriever
    labrador.speak()         # Output: Max says Woof!


if __name__ == "__main__":
    main()