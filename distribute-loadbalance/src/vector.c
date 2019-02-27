/*************************************************************************
	> File Name: vector.c
	> Author: perrynzhou
	> Mail: perrynzhou@gmail.com
	> Created Time: Fri 02 Jun 2017 09:01:30 PM HKT
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include "vector.h"
#define VECTOR_EXPAND_RATE (0.8)
#define VECTOR_DEFAULT_SIZE 1024
#define atomic_add(M,N) (__sync_add_and_fetch(&M,N))
#define atomic_sub(M,N) (__sync_sub_and_fetch(&M,N))
struct vector_s
{
	int64_t maxsize;
	int64_t cursize;
	int64_t oversize;
	void **item;				//save data
	pthread_mutex_t mutex;
	free_callback fcb;
	cmp_callback ccb;
};

inline int64_t vectorSize (vector * v)
{
	return v->cursize;
}

static void quickSort (vector * v, void *item[], int64_t left, int64_t right)
{
	if (left < right && v->ccb != NULL && v->cursize > 0)
	{
		int64_t i = left, j = right;
		void *tmp = item[left];
		while (i < j)
		{
			while (i < j && v->ccb (item[j], tmp))
			{
				j--;
			}
			if (i < j)
			{
				item[i++] = item[j];
			}
			while (i < j && !v->ccb (item[i], tmp))
			{
				i++;
			}
			if (i < j)
			{
				item[j--] = item[i];
			}
		}
		item[i] = tmp;
		quickSort (v, v->item, left, i - 1);
		quickSort (v, v->item, i + 1, right);
	}
}

inline void vectorSort (vector * v)
{
	quickSort (v, v->item, 0, v->cursize - 1);
}

static inline int vectorLock (vector * v)
{
	return pthread_mutex_lock (&v->mutex);
}

static inline int vectorUnlock (vector * v)
{
	return pthread_mutex_unlock (&v->mutex);
}

static inline int vector_mutex_init (vector * v)
{
	return pthread_mutex_init (&v->mutex, NULL);
}

static inline int vector_mutex_destroy (vector * v)
{
	return pthread_mutex_destroy (&v->mutex);
}

static void vectorExpand (vector * v)
{
	int64_t size = (v->maxsize * 2);
	v->item = realloc (v->item, sizeof (void *) * size);
	v->maxsize = size;
	v->oversize = (int64_t) (v->maxsize * VECTOR_EXPAND_RATE);
}

vector *vectorCreate (int64_t maxsize, free_callback fcb, cmp_callback ccb)
{
	vector *v = (vector *) malloc (sizeof (vector));
	assert (v != NULL);
	v->fcb = fcb;
	v->ccb = ccb;
	v->maxsize = maxsize;
	v->cursize =0;
	v->item = (void **) malloc (sizeof (void *) * v->maxsize);
	assert (v->item != NULL);
	v->oversize = (int64_t) (maxsize * VECTOR_EXPAND_RATE);
	for (uint32_t i = 0; i < v->maxsize; i++)
	{
		v->item[i] = NULL;
	}
	return v;
}

bool vectorAdd (vector * v, void *data)
{
	if (v != NULL && data != NULL)
	{
		if (v->cursize >= v->oversize)
		{
			vectorLock (v);
			vectorExpand (v);
			vectorUnlock (v);
		}
		v->item[v->cursize] = data;
		atomic_add (v->cursize, 1);
		return true;
	}
	return false;
}

bool vectorDel (vector * v, int64_t index)
{
	if (v != NULL && index >= 0 && index < v->cursize)
	{
		v->item[index] = NULL;
		atomic_sub (v->cursize, 1);
		return true;
	}
	return false;
}

void *vectorGet (vector * v, int64_t index)
{
	void *data = NULL;
	if (v != NULL && index < v->cursize)
	{
		data = v->item[index];
	}
	return data;
}

void vectorDestroy (vector * v)
{
	if (v != NULL)
	{
		for (uint32_t i = 0; i < v->cursize; i++)
		{
			if (v->fcb)
			{
				v->fcb (v->item[i]);
			}
		}
		free (v->item);
		free (v);
	}
}

#ifdef VECTOR_TEST
bool cmp (void *data1, void *data2)
{
	if (data1 != NULL && data2 != NULL)
	{
		int *v1 = (int *) data1;
		int *v2 = (int *) data2;
		return (*v1 >= *v2) ? true : false;
	}
	return false;
}

int main (int argc, char *args[])
{
	if (argc != 3)
	{
		fprintf (stdout, "%s [run_count] nosort \n", args[0]);
		fprintf (stdout, "%s [run_count] sort \n", args[0]);
		return 0;
	}
	int64_t tmp = atoi (args[1]);
	vector *v = vectorCreate (tmp, NULL, &cmp);
	fprintf (stdout, "vector maxsize=%d,cursize=%d,oversize=%d,t=%p\n", v->maxsize, v->cursize, v->oversize, v);
	int64_t *arr = (int64_t *) malloc (sizeof (int64_t) * tmp);
	fprintf (stdout, "---------------add--------------------\n");

	for (int64_t i = 0; i < tmp; i++)
	{
		arr[i] = rand () % tmp;
		vectorAdd (v, &arr[i]);
	}
	fprintf (stdout, "vector maxsize=%d,cursize=%d,oversize=%d,t=%p\n", v->maxsize, v->cursize, v->oversize, v);
	fprintf (stdout, "---------------sort and get--------------------\n");
	if (strncmp (args[2], "sort", 4) == 0)
	{
		vectorSort (v);
	}
	//vectorSort (v);
	for (int64_t i = 0; i < tmp; i++)
	{
		uint32_t *data = (uint32_t *) vectorGet (v, i);
		//fprintf (stdout, " i =%d,data=%d\n", i, *data);
	}
	fprintf (stdout, "------------------sub------------------\n");
	for (int64_t i = tmp - 1; i >= 0; i--)
	{
		vectorDel (v, i);
	}
	if (arr != NULL)
	{
		free (arr);
	}
	fprintf (stdout, "vector maxsize=%d,cursize=%d,oversize=%d,t=%p\n", v->maxsize, v->cursize, v->oversize, v);
	vectorDestroy (v);
	return 0;
}
#endif
