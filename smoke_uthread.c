#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

int total = 0;
uthread_cond_t paper_match;
uthread_cond_t paper_tobacco;
uthread_cond_t match_tobacco;

struct Agent {
  uthread_mutex_t mutex;
  uthread_cond_t  match;
  uthread_cond_t  paper;
  uthread_cond_t  tobacco;
  uthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex   = uthread_mutex_create();
  agent->paper   = uthread_cond_create (agent->mutex);
  agent->match   = uthread_cond_create (agent->mutex);
  agent->tobacco = uthread_cond_create (agent->mutex);
  agent->smoke   = uthread_cond_create (agent->mutex);
  return agent;
}

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
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  int i;
  uthread_mutex_lock (a->mutex);
    for (i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) { // how does this implementation enable agent to choose 2 random resource???
        VERBOSE_PRINT ("match available\n");
        uthread_cond_signal (a->match);
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        uthread_cond_signal (a->paper);
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        uthread_cond_signal (a->tobacco);
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      uthread_cond_wait (a->smoke);
    }
  uthread_mutex_unlock (a->mutex);
  return NULL;
}

void signal_smoker(int s){
	if(s==3){
		//signal match found
		uthread_cond_signal(paper_match);
		total = 0;
    }else if(s==6){
		//signal match found
		uthread_cond_signal(paper_tobacco);
		total = 0;
    }else if(s==5){
		//signal match found
		uthread_cond_signal(match_tobacco);
		total = 0;
	}
}

void* tobacco_owner (void* av){
  struct Agent* agent = av;
  uthread_mutex_lock(agent->mutex);
  for(;;){
    uthread_cond_wait(agent->tobacco);
    total = total + TOBACCO;
	//signal if match found
    signal_smoker(total);
  }
  uthread_mutex_unlock(agent->mutex);
}

void* paper_owner (void* av){
  struct Agent* agent = av;
  uthread_mutex_lock(agent->mutex);
  for(;;){
    uthread_cond_wait(agent->paper);
    total = total + PAPER;
	//signal if match found
    signal_smoker(total);
  }
  uthread_mutex_unlock(agent->mutex);
}

void* match_owner (void* av){
  struct Agent* agent = av;
  uthread_mutex_lock(agent->mutex);
  for(;;){
    uthread_cond_wait(agent->match);
    total = total+ MATCH;
	//signal if match found
    signal_smoker(total);
  }
  uthread_mutex_unlock(agent->mutex);
}

void* tabacco_smoker (void* av){
  struct Agent* agent = av;
  uthread_mutex_lock(agent->mutex);
  for(;;){
    uthread_cond_wait(paper_match);
	//signal that tobacco smoker is smoking
    uthread_cond_signal(agent->smoke);
    smoke_count [TOBACCO]++;
  }
  uthread_mutex_unlock(agent->mutex);
}

void* match_smoker (void* av){
  struct Agent* agent = av;
  uthread_mutex_lock(agent->mutex);
  for(;;){
    uthread_cond_wait(paper_tobacco);
	//signal that match smoker is smoking
    uthread_cond_signal(agent->smoke);
    smoke_count [MATCH]++;
  }
  uthread_mutex_unlock(agent->mutex);
}

void* paper_smoker (void* av){
  struct Agent* agent = av;
  uthread_mutex_lock(agent->mutex);
  for(;;){
    uthread_cond_wait(match_tobacco);
	//signal that paper smoker is smoking
    uthread_cond_signal(agent->smoke);
    smoke_count [PAPER]++;
  }
  uthread_mutex_unlock(agent->mutex);
}

int main (int argc, char** argv) {
  uthread_init (7);
  // Initialize objects
  struct Agent*  a = createAgent();
  //create condition variables
  paper_match = uthread_cond_create(a->mutex);
  paper_tobacco = uthread_cond_create(a->mutex);
  match_tobacco = uthread_cond_create(a->mutex);

  // create and join threads
  uthread_create (tobacco_owner, a);
  uthread_create (paper_owner, a);
  uthread_create (match_owner, a);
  uthread_create (tabacco_smoker, a);
  uthread_create (match_smoker, a);
  uthread_create (paper_smoker, a);

  uthread_join (uthread_create (agent, a), 0);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}