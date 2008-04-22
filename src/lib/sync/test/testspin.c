#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sync/spinlock.h>
#include <pthread.h>

int parse_arguments(int argc, char * argv[]);

extern int verbose_level;

int Data=0;
SPIN_LOCK DataLock;

#define WAIT_LOOP_COUNT (1000)
#define HOLD_LOOP_COUNT (10000)

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
		
void * thread_run(void * arg1);
int main(int argc, char * argv[])
{
	pthread_t thread1, thread2, thread3;
	parse_arguments(argc, argv); 
	
	pthread_create(&thread1, NULL, thread_run, (void *) 1 );
	pthread_create(&thread2, NULL, thread_run, (void *) 2 );
	pthread_create(&thread3, NULL, thread_run, (void *) 3 );
	
	pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	return 0;
}

void * thread_run(void * arg1)
{
	int i=100, j;
	int thread_num = (int) arg1;

	while (i > 0)
	{
		LOCK_DATA(thread_num);
	
		PRINT_DATA(thread_num);
		SET_DATA(thread_num);
	
		j=HOLD_LOOP_COUNT;
		while(j--);

		UNLOCK_DATA;
		
		j=WAIT_LOOP_COUNT;
		while(j--);
		
		i--;
	}
	return NULL;
}
