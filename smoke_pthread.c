#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

int total = 0;
pthread_cond_t paper_match=PTHREAD_COND_INITIALIZER;
pthread_cond_t paper_tobacco= PTHREAD_COND_INITIALIZER;
pthread_cond_t match_tobacco=PTHREAD_COND_INITIALIZER;

  pthread_mutex_t mutex;
  pthread_cond_t  match=PTHREAD_COND_INITIALIZER;
  pthread_cond_t  paper=PTHREAD_COND_INITIALIZER;
  pthread_cond_t  tobacco=PTHREAD_COND_INITIALIZER;
  pthread_cond_t  smoke=PTHREAD_COND_INITIALIZER;
/*

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex   = pthread_mutex_init(&mutex,NULL);
  agent->paper   = pthread_cond_create (agent->mutex);
  agent->match   = pthread_cond_create (agent->mutex);
  agent->tobacco = pthread_cond_create (agent->mutex);
  agent->smoke   = pthread_cond_create (agent->mutex);
  return agent;
}*/

/**
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4}; // why TOBACCO = 4 not 3 ???
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  int i;
  pthread_mutex_lock (&mutex);
    for (i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) { // how does this implementation enable agent to choose 2 random resource???
        VERBOSE_PRINT ("match available\n");
        pthread_cond_signal (&match);
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        pthread_cond_signal (&paper);
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        pthread_cond_signal (&tobacco);
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      pthread_cond_wait (&smoke,&mutex);
    }
  pthread_mutex_unlock (&mutex);
  return NULL;
}

void signal_smoker(int value){
	//MATCH + PAPER
	if(value==3){
		//signal match found
		pthread_cond_signal(&paper_match);
		total = 0;
    }else if(value==6){ //PAPER+TOBACCO
		//signal match found
		pthread_cond_signal(&paper_tobacco);
		total = 0;
    }else if(value==5){ //MATCH+TOBACCO
		//signal match found
		pthread_cond_signal(&match_tobacco);
		total = 0;
	}
}

void* tobacco_owner (void* av){
  pthread_mutex_lock(&mutex);
  for(;;){
    pthread_cond_wait(&tobacco,&mutex);
    total = total + TOBACCO;
	//signal if match found
    signal_smoker(total);
  }
  pthread_mutex_unlock(&mutex);
}

void* paper_owner (void* av){
  pthread_mutex_lock(&mutex);
  for(;;){
    pthread_cond_wait(&paper,&mutex);
    total = total + PAPER;
	//signal if match found
    signal_smoker(total);
  }
  pthread_mutex_unlock(&mutex);
}

void* match_owner (void* av){
  pthread_mutex_lock(&mutex);
  for(;;){
    pthread_cond_wait(&match,&mutex);
    total = total+ MATCH;
	//signal if match found
    signal_smoker(total);
  }
  pthread_mutex_unlock(&mutex);
}

void* tabacco_smoker (void* av){
  pthread_mutex_lock(&mutex);
  for(;;){
    pthread_cond_wait(&paper_match,&mutex);
	//signal that tobacco smoker is smoking
    pthread_cond_signal(&smoke);
    smoke_count [TOBACCO]++;
  }
  pthread_mutex_unlock(&mutex);
}

void* match_smoker (void* av){
  pthread_mutex_lock(&mutex);
  for(;;){
    pthread_cond_wait(&paper_tobacco,&mutex);
	//signal that match smoker is smoking
    pthread_cond_signal(&smoke);
    smoke_count [MATCH]++;
  }
  pthread_mutex_unlock(&mutex);
}

void* paper_smoker (void* av){
  pthread_mutex_lock(&mutex);
  for(;;){
    pthread_cond_wait(&match_tobacco,&mutex);
	//signal that paper smoker is smoking
    pthread_cond_signal(&smoke);
    smoke_count [PAPER]++;
  }
  pthread_mutex_unlock(&mutex);
}

int main (int argc, char** argv) {
  pthread_t t[7];
  
  pthread_mutex_init(&mutex,NULL);
  // create and join threads
  pthread_create (&t[0],NULL,&tobacco_owner,NULL);
  pthread_create (&t[1],NULL,&paper_owner, NULL);
  pthread_create (&t[2],NULL,&match_owner,NULL);
  pthread_create (&t[3],NULL,&tabacco_smoker,NULL);
  pthread_create (&t[4],NULL,&match_smoker, NULL);
  pthread_create (&t[5],NULL,&paper_smoker, NULL);

  pthread_join (pthread_create (&t[6],NULL,&agent,NULL), 0);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}