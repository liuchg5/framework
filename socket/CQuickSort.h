#ifndef CQUICKSORT_H
#define CQUICKSORT_H

#include "CSocketInfo.h"

class CQuickSort
{
public:
	int *a; // a数组用来排序
	int size, n;

	CSocketInfoList * const list;

	CQuickSort(int isize, CSocketInfoList * plist);
	~CQuickSort();

	void clear();
	void build_sort(struct timeval * now);




	void insert(int index);//EPOLL的index,session
	
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
