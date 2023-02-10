#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "flux.h"

// Structures

struct _funpar {
  void (*f)(void *);
  void *p;
  };

// Fonctions de lancement de flux

static void *_lancerFlux(void *arg){
struct _funpar *fp=arg;
fp->f(fp->p);
free(fp->p);
free(fp);
return NULL;
}

void lancerFlux(void (*f)(void *),void *p,int taille){
struct _funpar *fp=malloc(sizeof(struct _funpar));
fp->f=f;
if(taille>0){
  fp->p=malloc(taille);
  memcpy(fp->p,p,taille);
  }
else fp->p=NULL;
pthread_t tid;
if(pthread_create(&tid,NULL,_lancerFlux,fp)<0){
  perror("lancerFlux.pthread_create");
  exit(EXIT_FAILURE);
  }
if(pthread_detach(tid)<0){
  perror("lancerFlux.pthread_detach");
  exit(EXIT_FAILURE);
  }
}
