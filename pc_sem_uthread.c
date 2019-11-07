#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

uthread_sem_t lock;
uthread_sem_t producer_wait;
uthread_sem_t consumer_wait;

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
	uthread_sem_wait(producer_wait);
	uthread_sem_wait(lock);
	items++;
	assert(MAX_ITEMS>=items);
	histogram[items]=histogram[items]+1;
	uthread_sem_signal(consumer_wait);
	uthread_sem_signal(lock);
  }

  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
	uthread_sem_wait(consumer_wait);
	uthread_sem_wait(lock);
	assert(items>=1);
	items--;
	histogram[items]=histogram[items]+1;
	uthread_sem_signal(producer_wait);
	uthread_sem_signal(lock);
	
  }
  return NULL;
}

int main (int argc, char** argv) {
	int i;
  uthread_t t[4];

  uthread_init (4);
  lock=uthread_sem_create(1);
  producer_wait=uthread_sem_create(10);
  consumer_wait=uthread_sem_create(0);

  // TODO: Create Threads and Join
  for(i=0;i<NUM_PRODUCERS;i++){
	  t[i]=uthread_create(&producer,NULL);
  }
  for(i=0;i<NUM_CONSUMERS;i++){
	  t[i+2]=uthread_create(&consumer,NULL);
  }

	for(i=0;i<NUM_CONSUMERS+NUM_PRODUCERS;i++){
		uthread_join(t[i],NULL);
	}
  printf ("items value histogram:\n");
  int sum=0;
  for (i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
