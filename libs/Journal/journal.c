#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>        
#include <locale.h>

// Constantes

#define MAX_HEURE	1024
#define MAX_INFORMATION	1024

// Variable globale

static FILE *journal=NULL;
static int niveau=0;

// Fonctions de gestion de journal

void creationJournal(char *nom){
journal=fopen(nom,"a+");
}

void niveauJournal(int niv){
niveau=niv;
}

void ecritureJournal(int niv,char *information,...){
va_list arguments;
va_start(arguments,information);
if(journal==NULL) return;
time_t la=time(NULL);
char heure[MAX_HEURE];
if(la<0) strcpy(heure,"?");
else{ 
  struct tm *pla=localtime(&la);
  if(pla==NULL) strcpy(heure,"?");
  else strftime(heure,MAX_HEURE,"%c",pla);
  }
if(niv<=niveau){
  char infofmt[MAX_INFORMATION];
  sprintf(infofmt,information,arguments);
  fprintf(journal,"%s: %s\n",heure,infofmt);
  fflush(journal);
  }
}

void fermetureJournal(void){
if(journal!=NULL) fclose(journal);
}
