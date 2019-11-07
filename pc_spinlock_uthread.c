#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include "spinlock.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

spinlock_t lock;
//uthread_cond_t producer_wait;
//uthread_cond_t consumer_wait;

void* producer (void* v) {
	int i;
	spinlock_lock(&lock);
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
		while(items==10){
			producer_wait_count++;
			spinlock_unlock(&lock);
			spinlock_lock(&lock);
			//uthread_cond_wait(producer_wait);
		}
		assert(items<MAX_ITEMS);
		items=items+1;
		histogram[items]=histogram[items]+1;
		//uthread_cond_signal(consumer_wait);
		spinlock_unlock(&lock);
	}
	return NULL;
	
}

void* consumer (void* v) {
	int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
	spinlock_lock(&lock);
	while(items<1){
		consumer_wait_count++;
		//uthread_cond_wait(consumer_wait);
		spinlock_unlock(&lock);
		spinlock_lock(&lock);
	}
	assert(items>=1);
	items=items-1;
	histogram[items]=histogram[items]+1;
	//uthread_cond_signal(producer_wait);
	 spinlock_unlock(&lock);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);
  
  spinlock_create(&lock);
  
  //producer_wait=uthread_cond_create(lock);
  //consumer_wait=uthread_cond_create(lock);
  
  // TODO: Create Threads and Join 
  int i;
  for(i=0;i<NUM_CONSUMERS;i++){
	  t[i]=uthread_create(&consumer,NULL);
  }
  for(i=0;i<NUM_PRODUCERS;i++){
	  t[i+2]=uthread_create(&producer,NULL);
  }
  for(i=0; i<4;i++){
	uthread_join(t[i],NULL);
  }
  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}