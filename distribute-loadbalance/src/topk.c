/*************************************************************************
  > File Name: topk.c
  > Author:perrynzhou 
  > Mail:perrynzhou@gmail.com 
  > Created Time: 三  2/27 22:31:00 2019
 ************************************************************************/
#include <stdio.h>
void quickSort(int *arr, int low, int high, int k)
{
  int i = low, j = high;
  if (low < high)
  {
    int base = arr[i];
    while (i < j)
    {
      while (arr[j] < base && i < j)
      {
        j--;
      }
      if (i < j)
      {
        arr[i++] = arr[j];
      }
      while (arr[i] >= base && i < j)
      {
        i++;
      }
      if (i < j)
      {
        arr[j--] = arr[i];
      }
    }
    arr[i] = base;
    if ((j - low + 1) > k)
    {
      quickSort(arr, low, i - 1, k);
    }
    else
    {
      quickSort(arr, j + 1, high, k);
    }
  }
}
int findKthLargest(int *nums, int numsSize, int k)
{
  if (NULL == nums || numsSize <= 0 || k > numsSize)
  {
    return -1;
  }
  quickSort(nums, 0, numsSize - 1, k);
#ifdef TEST_DEBUG
  fprintf(stdout, "{");
  for (int i = 0; i < numsSize; i++)
  {
    if (i != (numsSize - 1))
    {
      fprintf(stdout, "%d,", nums[i]);
    }
    else
    {
      fprintf(stdout, "%d", nums[i]);
    }
  }
  fprintf(stdout, "}\n");
#endif
      return nums[k - 1];
}
/*
Example 1:
Input: [3,2,1,5,6,4] and k = 2
Output: 5

Example 2:
Input: [3,2,3,1,2,4,5,5,6] and k = 4
Output: 4


[3,2,3,1,2,4,5,5,6]
9
*/
int main(void)
{
  int arr1[] = {3, 2, 1, 5, 6, 4};
  int arr2[] = {3, 2, 3, 1, 21, 13,  45, 13, 25, 5, 5, 6, 11};
  int arr3[] = {3, 2, 3, 1, 2, 4, 5, 5, 6};
  int value1 = findKthLargest(arr1, 6, 2);
  fprintf(stdout, "results: top %d is %d\n", 2, value1);
  int value2 = findKthLargest(arr2, sizeof(arr2) / sizeof(int), 5);
  fprintf(stdout, "results: top %d is %d\n", 5, value2);
  int value3 = findKthLargest(arr3, sizeof(arr3) / sizeof(int), 4);
  fprintf(stdout, "results: top %d is %d\n", 4, value3);

  return 0;
}