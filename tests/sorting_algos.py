## Using this testcase, we show the following features:

##1. We show the correctness in 3 sorting algorithms: insertion sort, quick sort and bubble sort
##2. We show the use of lists of integers and how to pass them to functions, and 
##   even as objects attributes.
##3. We show the use of functions with return type as None.
##4. We show the use of functions with return type as int.
##5. We show the use of method functions and how to call them using objects of a class.

def partition(arr: list[int], low: int, high: int) -> int:
    pivot: int = arr[high]
    i: int = low - 1
    j:int =0
    k:int = 0
    for j in range(low, high):
        if arr[j]  <  pivot:
            i += 1
            k=arr[i]
            arr[i]=arr[j]
            arr[j]=k
    k=arr[i+1]
    arr[i+1]=arr[high]
    arr[high]=k
    return i + 1

def quicksort(arr: list[int], low: int, high: int) -> None:
    if low < high:
        pi: int = partition(arr, low, high)
        quicksort(arr, low, pi - 1)
        quicksort(arr, pi + 1, high)

class SortingAlgorithms:
    def __init__(self,n:int) :
        self.elements: list[int] = [17,9,3,4,6,7]
        self.n:int=n

    def insertion_sort(self):
        i:int=0
        k:int=self.n-1

        for i in range(k):
            key:int = self.elements[i+1]
            j:int = i 
            
            while j >= 0 and self.elements[j] > key:
                self.elements[j + 1] = self.elements[j]
                j = j- 1
            
            self.elements[j + 1] = key
    
    def quick_sort(self):
        quicksort(self.elements, 0, self.n - 1)

    def bubble_sort(self):
        i:int=0
        j:int=0
        k:int=0
        y: int = self.n
        for i in range(y):
            x: int = self.n - i - 1
            for j in range(x):
                if self.elements[j] > self.elements[j + 1]:
                    k = self.elements[j]
                    self.elements[j] = self.elements[j + 1]
                    self.elements[j + 1] = k

    def print_elements(self):
        i: int = 0
        for i in range(6):
            print(self.elements[i])

# Example usage:
def main():
    intsertion_sorting :SortingAlgorithms = SortingAlgorithms(6)
    print("Elements before insertion sort")
    intsertion_sorting.print_elements()
    intsertion_sorting.insertion_sort()
    print("Elements after insertion sort")
    intsertion_sorting.print_elements()

    quick_sorting:SortingAlgorithms = SortingAlgorithms(6)
    print("Elements before quick sort")
    quick_sorting.print_elements()
    quick_sorting.quick_sort()
    print("Elements after quick sort")
    quick_sorting.print_elements()
    
    bubble_sorting:SortingAlgorithms = SortingAlgorithms(6)
    print("Elements before bubble sort")
    bubble_sorting.print_elements()
    bubble_sorting.bubble_sort()
    print("Elements after bubble sort")
    bubble_sorting.print_elements()



if __name__ == "__main__":
    main()
