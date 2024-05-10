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

def main() -> None:
    arr: list[int] = [10, 7, 8, 9, 1, 5]
    n: int = len(arr)
    i:int=0
    quicksort(arr, 0, n - 1)
    for i in range(n):
        print(arr[i])

if __name__ == "__main__":
    main()