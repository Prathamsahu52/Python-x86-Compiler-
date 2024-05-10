def binarySearch(array: list[int], x: int, low: int, high: int) -> int:
  while low <= high:
    print("Hello")
    mid: int = low + high // 2   
  return -1


def main():
  array: list[int] = [3, 4, 5, 6, 7, 8, 9]
  result: int = binarySearch(array, 4, 0,  1)
  
  
  if result != -1:
    print("Element is present at index:")
    print(result)
  else:
    print("Element is not present")


if __name__ == "__main__":
  main()