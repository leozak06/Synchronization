#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


#define MAX_ITEMS 10 
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

pthread_mutex_t lock; //mutex declaration
pthread_cond_t producer_waiting = PTHREAD_COND_INITIALIZER; //convar initialization
pthread_cond_t consumer_waiting = PTHREAD_COND_INITIALIZER;

void* producer (void* v) {
	//lock thread
	pthread_mutex_lock(&lock); 
	int i;
	for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
		while(items==10){ //full -- producer gotta wait
			producer_wait_count++;
			pthread_cond_wait(&producer_waiting, &lock);
		}
		assert(MAX_ITEMS > items);
		items++; //if pool isnt full add items
		histogram[items]=histogram[items]+1;
		pthread_cond_signal(&consumer_waiting); //let consumer know item has been added
	}
	//unlock thread
	pthread_mutex_unlock(&lock);
	return NULL;
}

void* consumer (void* v) {
	//lock thread
	pthread_mutex_lock(&lock);
	int i;
	for (i=0; i<NUM_ITERATIONS; i++) {
    // TODO
		//if not empty consumer waits
		while(items<1){
			consumer_wait_count++;
			pthread_cond_wait(&consumer_waiting,&lock);
		}
		//when item added, consumer consumes item
		assert(items>=1);
		items--;
		histogram[items]=histogram[items]+1;
		pthread_cond_signal(&producer_waiting);
	}
	//unlock thread
	pthread_mutex_unlock(&lock);
	return NULL;
}

int main (int argc, char** argv) {
	pthread_t threads[4];
	int i,j;
	//initialize mutex
	if(pthread_mutex_init(&lock,NULL)!=0){
		printf("\n Mutex Initialization Failed\n");
		return 1;
	}
	//create producer threads
	for(i=0;i<NUM_PRODUCERS;i++){
		int err=pthread_create(&threads[i],NULL,producer,NULL);
		if(err!=0){
			printf("Error:Unable to create thread.\n");
		}
	}
	//create consumer threads
	for(i=0;i<NUM_CONSUMERS;i++){
		int err=pthread_create(&threads[i+2],NULL,consumer,NULL);
		if(err!=0){
			printf("Error: Unable to create thread.\n");
		}
	}
	//join producer and consumer threads
	for(j=0;j<NUM_PRODUCERS+NUM_CONSUMERS;j++){
		pthread_join(threads[j],NULL);
	}

	printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
	printf ("items value histogram:\n");
	int sum=0;
	for (i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
	}
	assert (sum == sizeof (threads) / sizeof (pthread_t) * NUM_ITERATIONS);
}