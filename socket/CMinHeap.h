#ifndef CMINHEAP_H
#define CMINHEAP_H

#include "CSocketInfo.h"

// i结点的父结点下标就为(i – 1) / 2。
// 它的左右子结点下标分别为2 * i + 1和2 * i + 2
// 注意使用最小堆排序后是递减数组，要得到递增数组，可以使用最大堆
class CMinHeap
{
public:
    int *array; //数组， 保存的是index
    int size, n;

    CSocketInfoList * const list;

    CMinHeap(int size, CSocketInfoList * List);
    ~CMinHeap();


    //在最小堆中加入新的数据nNum
    int insert(int index);// a[]表示堆基址，i表示插入的位置如果是新元素，则n

    void insert_just(int index);// a[]表示堆基址，i表示插入的位置如果是新元素，则n
    void del_just(int array_index);
    // //在最小堆中删除数
    // void deleteNumber(int a[], int n)
    // {
    //     Swap(a[0], a[n - 1]);
    //     MinHeapFixdown(a, 0, n - 1);
    // }
    // 注意使用最小堆排序后是递减数组
    void sort();
    //建立最小堆
    void build(); // ???有问题吗？




    void fixup(int a[], int i); // a[]表示堆基址，i表示插入的位置如果是新元素，则n
    //  从i节点开始调整,n为节点总数 从0开始计算 i节点的子节点为 2*i+1, 2*i+2
    void fixdown(int a[], int i, int n);// a[]表示堆基址

    int get_timeout_cnt(int index)
    {
    	return list->pv[index].tm_sec;
    }

    void swap(int i, int j)
	{
		int tmp = array[i];
		array[i] = array[j];
		array[j] = tmp;
	}
};


#endif
