
def bubbleSort():
  i: int = 0
  a: str = "abcd"
  
  array: list[int] = [64, 34, 25, 12, 22, 11, 90]
  
  for i in range(6):
    swapped: bool = False
    j: int = 0
    
    for j in range(0, 10):
      if array[j] > array[j + 1]:
        temp: int = array[j]
        array[j] = array[j + 1]
        array[j + 1] = temp
        swapped = True
      break
