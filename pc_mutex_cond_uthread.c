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

uthread_mutex_t mutex;
uthread_cond_t producer_wait;
uthread_cond_t consumer_wait;

void* producer (void* v) {
	uthread_mutex_lock(mutex);
	int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
		while(items==10){
			producer_wait_count++;
			uthread_cond_wait(producer_wait);
		}
		assert(MAX_ITEMS>items);
		items++;
		histogram[items]=histogram[items]+1;
		uthread_cond_signal(consumer_wait);
  }
  uthread_mutex_unlock(mutex);
  return NULL;
}

void* consumer (void* v) {
	uthread_mutex_lock(mutex);
	int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
	while(items<1){
		consumer_wait_count++;
		uthread_cond_wait(consumer_wait);
	}
	assert(items>=1);
	items--;
	histogram[items]=histogram[items]+1;
	uthread_cond_signal(producer_wait);
	
  }
  uthread_mutex_unlock(mutex);
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);
  
  int i,j;
  mutex=uthread_mutex_create();
  producer_wait=uthread_cond_create(mutex);
  consumer_wait=uthread_cond_create(mutex);
  
  
  // TODO: Create Threads and Join
  for(i=0;i<NUM_PRODUCERS;i++){
	  t[i]=uthread_create(&producer,NULL);
	 /* if(err!=0){
		  printf("Error: Unable to create uthread\n");
	  }*/
  }
  for(i=0;i<NUM_CONSUMERS;i++){
	  t[i+2]=uthread_create(&consumer,NULL);
	 /* if(err!=0){
		  printf("Error: Unable to create consumer threads");
	  }*/
  }
  for(j=0;j<NUM_PRODUCERS+NUM_CONSUMERS;j++){
	  uthread_join(t[j],NULL);
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