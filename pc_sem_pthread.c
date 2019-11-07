#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

sem_t lock;
sem_t producer_wait;
sem_t consumer_wait;

void* producer (void* v) {
	int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
	sem_wait(&producer_wait);
	sem_wait(&lock);
	items++;
	assert(MAX_ITEMS>=items);
	histogram[items]=histogram[items]+1;
	sem_post(&consumer_wait);
	sem_post(&lock);
  }

  return NULL;
}

void* consumer (void* v) {
	int i;
  for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
	sem_wait(&consumer_wait);
	sem_wait(&lock);
	assert(items<MAX_ITEMS);
	items--;
	histogram[items]=histogram[items]+1;
	sem_post(&producer_wait);
	sem_post(&lock);
	
  }
  return NULL;
}

int main (int argc, char** argv) {
	int i;
	pthread_t threads[4];
	if(sem_init(&lock,0,1)!=0){
		printf("Error: Unable to Initialize Semaphore\n");
	}
	if(sem_init(&producer_wait,0,1)!=0){
		printf("Error: Unable to Initialize Semaphore\n");
	}
	if(sem_init(&consumer_wait,0,1)!=0){
		printf("Error: Unable to Initialize Semaphore\n");
	}

  // TODO: Create Threads and Join
  for(i=0;i<NUM_PRODUCERS;i++){
	  int err=pthread_create(&threads[i],NULL,producer,NULL);
		if(err!=0){
			printf("Error:Unable to create thread.\n");
		}
  }
  for(i=0;i<NUM_CONSUMERS;i++){
	  int err=pthread_create(&threads[i+2],NULL,consumer,NULL);
		if(err!=0){
			printf("Error: Unable to create thread.\n");
		}
  }

	for(i=0;i<NUM_CONSUMERS+NUM_PRODUCERS;i++){
		pthread_join(threads[i],NULL);
	}
  printf ("items value histogram:\n");
  int sum=0;
  for (i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (threads) / sizeof (pthread_t) * NUM_ITERATIONS);
}