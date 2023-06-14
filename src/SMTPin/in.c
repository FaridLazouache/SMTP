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
printf("\nEntré dans gestionCourriel\n");
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
#ifdef DEVERMINE
printf("Domaine : %s\n",domaine);
#endif
if(mode!=1){ free(domaine); return; }
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
#ifdef DEVERMINE
  printf("\nss = %d\n", ss);
#endif
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
#ifdef DEVERMINE
  	printf("\nresultat = %d\n", resultat);
#endif
  if(resultat<0) ecritureJournal(JOURNIV_DEVERMINE,JOURNAL_MXHS,*p);
  else break;
  // Logguer l'erreur
  fclose(dialogue);
#ifdef DEVERMINE
  printf("\np = %s", *p);
#endif
  p++;
  }
}

// Fonctions de gestion des clients SMTP

void _fluxGestionSMTP(void *arg){
int sd=*(int *)arg;
#ifdef DEVERMINE
  		printf("\nFlux gestion : sd = %d\n", sd);
#endif

FILE *dialogue=fdopen(sd,"a+");
if(dialogue==NULL){
  perror("fluxGestionSMTP.fdopen");
	#ifdef DEVERMINE
  		printf("\nCan't open dialogue\n");
	#endif
  exit(EXIT_FAILURE);
  }
SMTP_gestion(dialogue,gestionCourriel,NULL);
}

int fluxGestionSMTP(int sd)
{
	#ifdef DEVERMINE
	printf("sd = %d", sd);
	#endif
	lancerFlux(_fluxGestionSMTP,&sd,sizeof(int));
	return 0;
}

// Fonction principale

int main(int argc,char *argv[]){
char *interface=NULL;
struct smtp_config config;
#ifdef DEVERMINE
	printf("\nanalyseArguments\n");
#endif
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
#ifdef DEVERMINE
	printf("s = %d", s);
#endif
if(s<0){
  fprintf(stderr,"Impossible d'occuper le port %s\n",config.port);
  exit(EXIT_FAILURE);
  }
#ifdef DEVERMINE
	printf("\nboucleServeur\n");
#endif
boucleServeur(s,fluxGestionSMTP);
#ifdef DEVERMINE
	printf("\necritureJournal\n");
#endif
ecritureJournal(JOURNIV_INFO,JOURNAL_FIN);
#ifdef DEVERMINE
	printf("\nfermetureJournal\n");
#endif
fermetureJournal();
return 0;
}
