#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "reseau.h"
#include "smtp.h"
#include "flux.h"
#include "journal.h"

#include "args.h"

// Constantes

#define TAILLE_FILE		100
#define MAX_ERREUR		2048

#define	JOURNIV_INFO		10
#define	JOURNIV_DEVERMINE	20
#define JOURNIV_ERREUR 0

#define JOURNAL_DEBUT	"Serveur SMTP d'envoi"
#define JOURNAL_FIN	"Sortie du serveur"
#define JOURNAL_MXHS	"Serveur MX non joignable (%s)"
#define JOURNAL_ERREUR "\nErreur de connexion au serveur\n"

// Fonction de gestion des courriels recus

void gestionCourriel(struct courriel *donnees,void *supplement){
#ifdef DEVERMINE
afficheCourriel(stdout,donnees,0);
#endif
char *domaine=malloc(strlen(donnees->destinataire));
int i,j=0;;
unsigned char mode=0;
for(i=0;i<strlen(donnees->destinataire);i++){
  if(donnees->destinataire[i]=='>') break;
  if(mode==1) domaine[j++]=donnees->destinataire[i];
  if(donnees->destinataire[i]=='@') mode=1;
  }
domaine[j]='\0';
if(mode!=1){ free(domaine); return; }
#ifdef DEVERMINE
printf("Domaine : %s\n",domaine);
#endif
char **lesmx=chercherMX(domaine);
if(lesmx!=NULL) free(domaine);
else{
  lesmx=calloc(2,sizeof(char *));
  lesmx[0]=domaine;
  }
char **p=lesmx;
while(*p!=NULL){
#ifdef DEVERMINE
  printf("Serveur : %s\n",*p);
#endif
  int ss=connexionServeur(*p,SMTP_PORT_DEFAULT);
  if(ss == -1)
  {
    ecritureJournal(JOURNIV_ERREUR, JOURNAL_ERREUR);
    continue;
  }
  FILE *dialogue=fdopen(ss,"a+");
  if(dialogue==NULL){
    perror("gestionCourriel.fdopen");
    exit(EXIT_FAILURE);
  }
  char erreur[MAX_ERREUR];
  int resultat=SMTP_dialogue(dialogue,donnees,erreur,MAX_ERREUR);
  if(resultat<0) ecritureJournal(JOURNIV_DEVERMINE,JOURNAL_MXHS,*p);
  // Logguer l'erreur
  fclose(dialogue);
  p++;
  }
}

// Fonctions de gestion des clients SMTP

void _fluxGestionSMTP(void *arg){
int sd=*(int *)arg;
FILE *dialogue=fdopen(sd,"a+");
if(dialogue==NULL){
  perror("fluxGestionSMTP.fdopen");
  exit(EXIT_FAILURE);
  }
SMTP_gestion(dialogue,gestionCourriel,NULL);
}

int fluxGestionSMTP(int sd){
lancerFlux(_fluxGestionSMTP,&sd,sizeof(int));
return 0;
}

// Fonction principale

int main(int argc,char *argv[]){
char *interface=NULL;
struct smtp_config config;
analyseArguments(argc,argv,&config);
if(config.journal[0]!='\0') creationJournal(config.journal);
niveauJournal(config.niveau);
ecritureJournal(JOURNIV_INFO,JOURNAL_DEBUT);
if(config.port[0]=='\0') strcpy(config.port,SMTP_PORT_DEFAULT);
if(config.local==1) interface=SMTP_LOCAL_INTERFACE;
#ifdef DEVERMINE
printf("Port : %s\n",config.port);
if(interface!=NULL) printf("Interface : %s\n",interface);
#endif
int s=initialisationServeur(config.port,TAILLE_FILE,interface);
if(s<0){
  fprintf(stderr,"Impossible d'occuper le port %s\n",config.port);
  exit(EXIT_FAILURE);
  }
boucleServeur(s,fluxGestionSMTP);
ecritureJournal(JOURNIV_INFO,JOURNAL_FIN);
fermetureJournal();
return 0;
}
