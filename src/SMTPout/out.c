#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "reseau.h"
#include "smtp.h"
#include "flux.h"
#include "journal.h"
#include "args.h"

// Constantes

#define TAILLE_FILE             100
#define MAX_ERREUR              2048
#define MAX_CHEMIN              2048
#define MAX_TAILLE_ADRESSE	128
#define MAX_ADRESSES		128
#define MAX_LIGNE		1024

#define MUTEX_COMPTEUR		0

#define JOURNIV_ERREUR		0
#define JOURNIV_ALERTE		5
#define JOURNIV_INFO		10
#define JOURNIV_DEVERMINE	20

#define JOURNAL_DEBUT		"Serveur SMTP de stockage"
#define JOURNAL_FIN		"Sortie du serveur"
#define JOURNAL_INCONNU		"Destinataire inconnu '%s'"
#define JOURNAL_OUVERTURE	"Ouverture en écriture impossible '%s'"
#define JOURNAL_ECRITURE	"Ecriture impossible '%s'"

#define MAILDIR_DFLTBASE	"/tmp"
#define MAILDIR_TMP		"tmp"
#define MAILDIR_NEW		"new"

// Structures

typedef struct {
  char adresse[MAX_TAILLE_ADRESSE];
  char *id;
  } adrVersID;

// Variables globales

long int messages;
char *dossier;
pid_t pid;
adrVersID adresses[MAX_ADRESSES];

// Analyse de la carte des utilisateurs
// Syntaxe d'une ligne de la carte :
// identifiant : adresse_courriel, adresse_courriel, ...

void chargeCarteUtilisateurs(char *carte){
char ligne[MAX_LIGNE];
char id[MAX_LIGNE];
char reste[MAX_LIGNE];
FILE *f=fopen(carte,"r");
if(f==NULL){
  fprintf(stderr,"Carte des adresses non lisible '%s'\n",carte);
  exit(EXIT_FAILURE);
  }
int i=0;
while(fgets(ligne,MAX_LIGNE,f)!=NULL){
  sscanf(ligne,"%[^ :] : %[^\n]",id,reste);
  char *copie=strdup(id);
  char *t=strtok(reste," ,");
  while(t!=NULL){
    strcpy(adresses[i].adresse,t);
    adresses[i].id=copie;
    t=strtok(NULL," ,");
    i++;
    if(i==MAX_ADRESSES-1) break;
    }
  }
adresses[i].id=NULL;
fclose(f);
}

// Affiche la carte des utilisateurs

void afficheCarteUtilisateur(void){
int i=0;
while(adresses[i].id!=NULL){
  printf("[%s] %s\n",adresses[i].id,adresses[i].adresse);
  i++;
  }
}

// Recherche un utilisateur

char *scanCarteUtilisateur(char *courriel){
int i=0;
while(adresses[i].id!=NULL){
  if(strcasecmp(adresses[i].adresse,courriel)) return adresses[i].id;
  i++;
  }
return NULL;
}

// Relache la carte des utilisateurs

void relacheCarteUtilisateur(void){
int i=0;
while(adresses[i].id!=NULL){
  if(adresses[i].id!=NULL) free(adresses[i].id);
  i++;
  }
}

// Fonction d'initialisation pour le stockage 

void stockeInit(struct smtp_config *config){
if(strlen(config->dossier)>0) dossier=config->dossier;
else dossier=MAILDIR_DFLTBASE;
if(strlen(config->carte)>0) chargeCarteUtilisateurs(config->carte);
afficheCarteUtilisateur();
pid=getpid();
}

// Extraction de l'utilisateur à partir de l'adresse de courriel

char *adresseVersUtilisateur(char *destinataire){
char *adresse=malloc(strlen(destinataire));
int i,j=0;;
unsigned char mode=0;
for(i=0;i<strlen(adresse);i++){
  if(destinataire[i]=='>') break;
  if(mode==1) adresse[j++]=destinataire[i];
  if(destinataire[i]=='<') mode=1;
  }
adresse[j]='\0';
char *id=scanCarteUtilisateur(adresse);
free(adresse);
return id;
}

// Fonction de gestion des courriels recus

void gestionCourriel(struct courriel *donnees,void *supplement){
  #ifdef DEVERMINE
  afficheCourriel(stdout,donnees,0);
  #endif
  char *id=adresseVersUtilisateur(donnees->destinataire);
  if(id==NULL){
    ecritureJournal(JOURNIV_ALERTE,JOURNAL_INCONNU,donnees->destinataire);
    return;
    }
  P(MUTEX_COMPTEUR);
  messages++;
  V(MUTEX_COMPTEUR);
  char fichier[MAX_CHEMIN];
  sprintf(fichier,"%s/%s/%s/%ld_%010d_%010ld",dossier,id,MAILDIR_TMP,time(NULL),pid,messages);
  FILE *f=fopen(fichier,"w");
  if(f==NULL){
    ecritureJournal(JOURNIV_ERREUR,JOURNAL_OUVERTURE,fichier);
    return;
    }
  int nb=fwrite(donnees->corps,donnees->taille,1,f);
  if(nb!=1){
    ecritureJournal(JOURNIV_ERREUR,JOURNAL_ECRITURE,fichier);
    return;
    }
  fclose(f);
  char newfile[MAX_CHEMIN];
  sprintf(newfile,"%s/%s/%s/%ld_%010d_%010ld",dossier,id,MAILDIR_NEW,time(NULL),pid,messages);
  rename(fichier, newfile);
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
if(config.local==1) 
    interface=SMTP_LOCAL_INTERFACE;
else
    interface = config.interface;

int statut=mutex_init(1);
stockeInit(&config);
if(statut<0){
  fprintf(stderr,"Erreur dans l'initialisation des mutex\n");
  exit(EXIT_FAILURE);
  }
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
mutex_fin();
return 0;
}

