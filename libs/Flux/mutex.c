#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "flux.h"

pthread_mutex_t *mutexes;
int mutex_nb=0;

int mutex_init(int nb){
int status;
mutexes=malloc(sizeof(pthread_mutex_t)*nb);
if(mutexes==NULL) return -1;
mutex_nb=nb;
int i;
for(i=0;i<nb;i++){
  status=pthread_mutex_init(mutexes+i,NULL);
  if(status!=0) return -1;
  }
return 0;
}

int mutex_fin(void){
int status;
int i;
for(i=0;i<mutex_nb;i++){
  status=pthread_mutex_destroy(mutexes+i);
  if(status!=0) return -1;
	}
  free(mutexes);
return 0;
}

int P(int m){
int status=pthread_mutex_lock(mutexes+m);
return (status!=0)?-1:0;
}

int V(int m){
int status=pthread_mutex_unlock(mutexes+m);
return (status!=0)?-1:0;
}
