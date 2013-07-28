#include "CQuickSort.h"

#include <stdio.h>


CQuickSort::CQuickSort(int isize, CSocketInfoList *plist): list(plist)
{
    n = 0;
    size = isize;
    a = new int[size];
    if (a == NULL)
    {
        fprintf(stderr, "Err: CQuickSort::CQuickSort()\n" );
        fprintf(stderr, "Err: new failed!!! \n" );
    }
}

CQuickSort::~CQuickSort()
{
    delete [] a;
}

void CQuickSort::insert(int index)
{
    if (n == size)
    {
        fprintf(stderr, "Err: CQuickSort::insert()\n" );
        fprintf(stderr, "Err: full !!! \n" );
        return;
    }
    a[n++] = index;
}

void CQuickSort::del(int a_index)//数组a的下标
{
    if (a_index == n - 1)//最后一个元素
    {
        n--;
    }
    else
    {
        for (int i = a_index; i < n - 1; i++)
        {
            a[i] = a[i + 1];
        }
        n--;
    }
}

void CQuickSort::sort()
{
    qsort(a, 0, n - 1);
}


void CQuickSort::qsort(int v[], int L, int R)
{
    if (R - L < 5) // 使用直接插入排序
    {
        isort(v, L, R);
        return;
    }

    if (L < R)
    {
        swap(L, (L + R) / 2); //将中间的这个数和第一个数交换
        int i = L, j = R;
        int tmp = v[i];
        int x = get_tm_sec(tmp);
        while (i < j)
        {
            while (i < j && get_tm_sec(v[j]) >= x)
                j--;
            if (i < j)
                v[i++] = v[j];

            while (i < j && get_tm_sec(v[i]) < x)
                i++;
            if (i < j)
                v[j--] = v[i];

        }
        v[i] = tmp;
        qsort(v, L, i - 1);
        qsort(v, i + 1, R);
    }
}

void CQuickSort::isort(int v[], int L, int R)
{
    int i, j;
    for (i = L + 1; i <= R; i++)
        for (j = i - 1; j >= L && get_tm_sec(v[j]) > get_tm_sec(v[j + 1]); j--)
            swap(j, j + 1);
}