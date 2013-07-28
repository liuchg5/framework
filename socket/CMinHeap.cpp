#include "CMinHeap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CMinHeap::CMinHeap(int size, CSocketInfoList *List): list(List)
{
    n = 0;
    this->size = size;
    array = new int[size];
    if (array == NULL)
    {
        fprintf(stderr, "Err:CMinHeap::CMinHeap() \n" );
        fprintf(stderr, "Err: new failed!!! \n" );
    }
}

CMinHeap::~CMinHeap()
{
    delete [] array;
}



int CMinHeap::insert(int index)
{
    if (n == size)
    {
        fprintf(stderr, "Err: CMinHeap::insert() \n" );
        fprintf(stderr, "Err: full !!! \n" );
        return -1;
    }
    array[n] = index;
    fixup(array, n);
    n++;
    return 0;
}


void CMinHeap::insert_just(int index)
{
    if (n == size)
    {
        fprintf(stderr, "Err: CMinHeap::insert() \n" );
        fprintf(stderr, "Err: full !!! \n" );
        return;
    }
    array[n++] = index;
}
void CMinHeap::del_just(int array_index)
{
    if (array_index == n - 1)//最后一个元素
    {
        n--;
    }
    else
    {
        for (int i = array_index; i < n - 1; i++)
        {
            array[i] = array[i + 1];
        }
        n--;
    }
}


void CMinHeap::sort()
{
    int tmp;
    for (int i = n - 1; i >= 1; i--)
    {
        tmp = array[i]; array[i] = array[0]; array[0] = tmp;
        fixdown(array, 0, i);
    }
}

void CMinHeap::build() // ???有问题吗？
{
    for (int i = n / 2 - 1; i >= 0; i--)
    {
        fixdown(array, i, n);
    }
}


//-----------------------------------------

void CMinHeap::fixup(int a[], int i) // a[]表示堆基址，i表示插入的位置如果是新元素，则n
{
    int j;
    int tmp = a[i];
    int val_i = get_timeout_cnt(a[i]);
    int val_j;
    j = (i - 1) / 2;      //父结点
    while (j >= 0 && i != 0)
    {
        val_j = get_timeout_cnt(a[j]);
        if (val_j <= val_i) // 只有比较的时候是比较timeout_cnt
            break;

        a[i] = a[j]; //把较大的子结点往下移动,替换它的子结点
        i = j;
        j = (i - 1) / 2;
    }
    a[i] = tmp;
}
void CMinHeap::fixdown(int a[], int i, int n)// a[]表示堆基址
{
    int j;
    int tmp = a[i];
    int val_i = get_timeout_cnt(a[i]);
    int val_j;

    j = 2 * i + 1; // 子节点
    while (j < n)
    {
        val_j = get_timeout_cnt(a[j]);
        if (j + 1 < n &&
                get_timeout_cnt(a[j + 1]) < val_j) //在左右孩子中找最小的
            j++;

        if (val_j >= val_i)
            break;

        a[i] = a[j];     //把较小的子结点往上移动,替换它的父结点
        i = j;
        j = 2 * i + 1;
    }
    a[i] = tmp;
}

