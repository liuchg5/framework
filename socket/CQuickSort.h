#ifndef CQUICKSORT_H
#define CQUICKSORT_H

#include "CSocketInfo.h"

class CQuickSort
{
public:
	int *a;
	int size, n;

	CSocketInfoList * const list;

	CQuickSort(int isize, CSocketInfoList * plist);
	~CQuickSort();

	void insert(int index);//EPOLL的index,session
	void del(int a_index);//数组a的下标
	void sort();


	
	void qsort(int v[], int L, int R);

	void isort(int v[], int L, int R);

	int get_tm_sec(int index)
	{
		return list->pv[index].tm_sec;
	}
	void swap(int i, int j)
	{
		int tmp = a[i];
		a[i] = a[j];
		a[j] = tmp;
	}
};

#endif
