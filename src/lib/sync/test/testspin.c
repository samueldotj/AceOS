#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sync/spinlock.h>
#include <pthread.h>

int parse_arguments(int argc, char * argv[]);

extern int verbose_level;

/*! This data will be accessed by different threads*/
int Data=0;
/*! The above data is protected by this lock*/
SPIN_LOCK DataLock;

/*! After acquiring the lock, threads will spin for the following number of times before releasing the lock*/
#define HOLD_LOOP_COUNT 	(100000)
/*! After releasing the lock, threads will spin for the following number of times before trying to get the lock again*/
#define WAIT_LOOP_COUNT 	(100)

#define LOCK_DATA(t)						\
		if ( SpinLock(&DataLock) )			\
		{									\
			printf("Spinlock Timeout %d\n",t );	\
			exit(1);						\
		}			
#define UNLOCK_DATA	SpinUnlock( &DataLock )

#define PRINT_DATA(t) if (verbose_level > 1) printf("Thread %d :: data %d\n", t, Data);
#define SET_DATA(t) Data = t; 
#define CHECK_DATA(t) 	\
	if ( Data != t )	\
	{					\
		printf("Mismatch %d %d", t, Data);\
		exit(1);		\
	}

void SpinLockTimeout(SPIN_LOCK_PTR pLockData, void * caller)
{
	printf("SpinLock Timeout");
	exit(1);
}
void * thread_run(void * arg1);
int main(int argc, char * argv[])
{
	pthread_t thread1, thread2, thread3, thread4, thread5, thread6, thread7, thread8;
	parse_arguments(argc, argv); 
	
	pthread_create(&thread1, NULL, thread_run, (void *) 1 );
	pthread_create(&thread2, NULL, thread_run, (void *) 2 );
	pthread_create(&thread3, NULL, thread_run, (void *) 3 );
	pthread_create(&thread4, NULL, thread_run, (void *) 4 );
	pthread_create(&thread5, NULL, thread_run, (void *) 5 );
	pthread_create(&thread6, NULL, thread_run, (void *) 6 );
	pthread_create(&thread7, NULL, thread_run, (void *) 7 );
	pthread_create(&thread8, NULL, thread_run, (void *) 8 );
	
	pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	return 0;
}

void * thread_run(void * arg1)
{
	int i=1000, j;
	int thread_num = (int) arg1;

	while (i > 0)
	{
		/*! lock the data*/
		LOCK_DATA(thread_num);
	
		/*! print the value*/
		PRINT_DATA(thread_num);
		
		/*! set the value to current thread num*/
		SET_DATA(thread_num);
	
		/*! hold the lock*/
		j=HOLD_LOOP_COUNT;
		while(j--);
		
		/*! check for lock leakage*/
		CHECK_DATA(thread_num);

		/*! unlock the data*/
		UNLOCK_DATA;
		
		/*! wait for some time*/
		j=WAIT_LOOP_COUNT;
		while(j--);
		
		i--;
	}
	return NULL;
}
