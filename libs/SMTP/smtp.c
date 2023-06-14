#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "smtp.h"
#include "smtp_private.h"

// Constantes 

#define MAX_COMMANDE		16
#define MAX_LIGNE		1024

#define GESTION_OK		0
#define GESTION_ERREUR		1
#define GESTION_STOP		-1
#define GESTION_TRAITE		-2

#define AFFICHE_COURRIEL_DEBUT	3
#define AFFICHE_COURRIEL_FIN	3

// Structure des commandes

struct commandes {
  char texte[MAX_COMMANDE];
  int (*traitement)(char *,FILE *,struct courriel *);
  };

// Fonctions  gestion des commandes SMTP

static int gestion_HELO(char *ligne,FILE *client,struct courriel *donnees){
char cmd[MAX_LIGNE];
char arg[MAX_LIGNE];
char suite[MAX_LIGNE];
int statut=sscanf(ligne,"%4s %s %s",cmd,arg,suite);
if(statut!=2){
  if(fprintf(client,"%03d %s\r\n",ERREUR_HELO_CODE,ERREUR_HELO_TEXTE)<0) return GESTION_STOP;
  return GESTION_ERREUR;
  }
strncpy(donnees->hello,arg,MAX_HELLO-1);
if(fprintf(client,"%03d %s\r\n",SUCCES_HELO_CODE,SUCCES_HELO_TEXTE)<0) return GESTION_STOP;
return GESTION_OK;
}

static int gestion_MAIL(char *ligne,FILE *client,struct courriel *donnees){
char cmd[MAX_LIGNE];
char tag[MAX_LIGNE];
char arg[MAX_LIGNE];
char suite[MAX_LIGNE];
char format[MAX_LIGNE];
int statut=sscanf(ligne,"%4s %s %s %s",cmd,tag,arg,suite);
#ifdef DEVERMINE
	printf("\nstatut = %d\n", statut);
#endif
if(statut!=3 || strcasecmp(tag,MAIL_TAG)!=0){

  	sprintf(format,"%%4s %s%%s %%s", MAIL_TAG);
#ifdef DEVERMINE
	printf("\nformat = %s\n", format);
#endif
	int statut2=sscanf(ligne,format,cmd,arg,suite);
#ifdef DEVERMINE
	printf("\nstatut2 = %d\n", statut2);
#endif
	if(statut2 !=2){ 
		if(fprintf(client,"%03d %s\r\n",ERREUR_MAIL_CODE,ERREUR_MAIL_TEXTE)<0) return GESTION_STOP;
#ifdef DEVERMINE
		printf("\nclient = %s\n", client);
#endif
  		return GESTION_ERREUR;}
}
  
if(donnees->hello[0]=='\0'){
  if(fprintf(client,"%03d %s\r\n",ERRORD_MAIL_CODE,ERRORD_MAIL_TEXTE)<0) return GESTION_STOP;
  return GESTION_ERREUR;
  }
strncpy(donnees->expediteur,arg,MAX_ADRESSE_COURRIEL-1);
if(fprintf(client,"%03d %s\r\n",SUCCES_MAIL_CODE,SUCCES_MAIL_TEXTE)<0) return GESTION_STOP;
return GESTION_OK;
}

static int gestion_RCPT(char *ligne,FILE *client,struct courriel *donnees){
char cmd[MAX_LIGNE];
char tag[MAX_LIGNE];
char arg[MAX_LIGNE];
char suite[MAX_LIGNE];
int statut=sscanf(ligne,"%4s %s %s %s",cmd,tag,arg,suite);
if(statut!=3 || strcasecmp(tag,RCPT_TAG)!=0){
  if(fprintf(client,"%03d %s\r\n",ERREUR_RCPT_CODE,ERREUR_RCPT_TEXTE)<0) return GESTION_STOP;
  return GESTION_ERREUR;
  }
if(donnees->hello[0]=='\0'){
  if(fprintf(client,"%03d %s\r\n",ERRORD_RCPT_CODE,ERRORD_RCPT_TEXTE)<0) return GESTION_STOP;
  return GESTION_ERREUR;
  }
strncpy(donnees->destinataire,arg,MAX_ADRESSE_COURRIEL-1);
if(fprintf(client,"%03d %s\r\n",SUCCES_RCPT_CODE,SUCCES_RCPT_TEXTE)<0) return GESTION_STOP;
return GESTION_OK;
}

static int gestion_DATA(char *ligne,FILE *client,struct courriel *donnees){
char cmd[MAX_LIGNE];
char suite[MAX_LIGNE];
int statut=sscanf(ligne,"%4s %s",cmd,suite);
if(statut!=1){
  if(fprintf(client,"%03d %s\r\n",ERREUR_DATA_CODE,ERREUR_DATA_TEXTE)<0) return GESTION_STOP;
  return GESTION_ERREUR;
  }
if(donnees->expediteur[0]=='\0' || donnees->destinataire[0]=='\0'){
  if(fprintf(client,"%03d %s\r\n",ERRORD_DATA_CODE,ERRORD_DATA_TEXTE)<0) return GESTION_STOP;
  return GESTION_ERREUR;
  }
if(fprintf(client,"%03d %s\r\n",CONTINUE_DATA_CODE,CONTINUE_DATA_TEXTE)<0) return GESTION_STOP;
donnees->corps=malloc(DATA_BLOCK);
donnees->taille=0;
int taille_allouee=DATA_BLOCK;
char texte[MAX_LIGNE];
while(fgets(texte,MAX_LIGNE,client)!=NULL){
  if(strlen(texte)<=3 && texte[0]=='.'){
    if(fprintf(client,"%03d %s\r\n",SUCCES_DATA_CODE,SUCCES_DATA_TEXTE)<0) return GESTION_STOP;
    return GESTION_TRAITE;
    } 
  if(taille_allouee-donnees->taille<=strlen(texte)){
    taille_allouee += DATA_BLOCK;
    donnees->corps=realloc(donnees->corps,taille_allouee);
    }
  strcpy(donnees->corps+donnees->taille,texte);
  donnees->taille += strlen(texte);
  }
return GESTION_STOP;
}

static int gestion_QUIT(char *ligne,FILE *client,struct courriel *donnees){
char cmd[MAX_LIGNE];
char suite[MAX_LIGNE];
int statut=sscanf(ligne,"%4s %s",cmd,suite);
if(statut!=1){
  if(fprintf(client,"%03d %s\r\n",ERREUR_QUIT_CODE,ERREUR_QUIT_TEXTE)<0) return GESTION_STOP;
  return GESTION_ERREUR;
  }
if(fprintf(client,"%03d %s\r\n",SUCCES_QUIT_CODE,SUCCES_QUIT_TEXTE)<0) return GESTION_STOP;
return GESTION_STOP;
}

// Fonction SMTP principale

struct commandes cmd_smtp[]={
  {"EHLO",gestion_HELO}, // changed "HELO" to EHLO"
  {"MAIL",gestion_MAIL},
  {"RCPT",gestion_RCPT},
  {"DATA",gestion_DATA},
  {"QUIT",gestion_QUIT},
  {"",NULL}
  };

struct courriel *SMTP_gestion(FILE *dialogue,void (*traitement)(struct courriel *,void *),void *tparam){
char ligne[MAX_LIGNE];
char cmd[MAX_LIGNE];
if(fprintf(dialogue,"%03d %s\r\n",ACCUEIL_CODE,ACCUEIL_TEXTE)<0){ fclose(dialogue); return NULL; }
struct courriel *donnees=calloc(1,sizeof(struct courriel));
donnees->taille=-1; 
while(fgets(ligne,MAX_LIGNE,dialogue)!=NULL){
  sscanf(ligne,"%4s",cmd);
  struct commandes *p=cmd_smtp;
  while(p->traitement!=NULL){
    if(strcasecmp(p->texte,cmd)==0){
#ifdef DEVERMINE
printf("Commande %s\n",cmd);
#endif 
      int statut=p->traitement(ligne,dialogue,donnees);
      if(statut==GESTION_STOP) goto fin;
      if(statut==GESTION_TRAITE){
        traitement(donnees,tparam);
        if(donnees->corps!=NULL) free(donnees->corps);
        memset(donnees,0,sizeof(struct courriel));
        donnees->taille=-1;
        }
      }
    p++;
    }
  }
fin:
fclose(dialogue);
return 0;
}

// Fonctions de communication avec un serveur SMTP

static int retour_generique(FILE *dialogue,int succes,char *erreur,int taille)
{
	char ligne[MAX_LIGNE];
	//while(fgets(ligne, MAX_LIGNE, dialogue) != '-') // changed fgets to consider hyphen
	//{
		if(fgets(ligne,MAX_LIGNE,dialogue)==NULL) return -1;
		ligne[MAX_LIGNE-1]='\0';
		
		int code;
		int statut=sscanf(ligne,"%d",&code);
		#ifdef DEVERMINE
			printf("code = %d\n", code);
			printf("statut = %d\n", statut);
			printf("succes = %d\n", succes);
		#endif
		if(statut==1 && code!=succes) strncpy(erreur,ligne,taille-1);
		return (statut==1 && code==succes)?0:-1;
	//}
}


static int dialogue_generique(FILE *dialogue,char *commande,char *arguments,int succes,char *erreur,int taille){
char envoi[MAX_LIGNE];
if(commande!=NULL){
  sprintf(envoi,"%s %s\r\n",commande,arguments);
  if(fprintf(dialogue,envoi)<0) return -1;
  }
return retour_generique(dialogue,succes,erreur,taille);
}

static inline int dialogue_connexion(FILE *dialogue,char *erreur,int taille)
{
	return dialogue_generique(dialogue,NULL,NULL,ACCUEIL_CODE,erreur,taille);
}

static inline int dialogue_QUIT(FILE *dialogue,char *erreur,int taille){
return dialogue_generique(dialogue,NULL,NULL,SUCCES_QUIT_CODE,erreur,taille);
}

static inline int dialogue_EHLO(FILE *dialogue,char *identification,char *erreur,int taille){
return dialogue_generique(dialogue,"EHLO",identification,SUCCES_HELO_CODE,erreur,taille); // changed "HELO" to "EHLO"
}

static inline int dialogue_MAIL(FILE *dialogue,char *expediteur,char *erreur,int taille){
return dialogue_generique(dialogue,"MAIL FROM:",expediteur,SUCCES_MAIL_CODE,erreur,taille);
}

static inline int dialogue_RCPT(FILE *dialogue,char *destinataire,char *erreur,int taille){
return dialogue_generique(dialogue,"RCPT TO:",destinataire,SUCCES_RCPT_CODE,erreur,taille);
}

static int dialogue_DATA(FILE *dialogue,char *corps,char *erreur,int taille)
{
	int code;
// FAIRE APPEL A DIALOGUE_GENERIQUE POUR ENVOYER DATA AVANT D'ENVOYER LE CORPS ET LE POINT
	code = dialogue_generique(dialogue, "DATA", "", CONTINUE_DATA_CODE, erreur, taille);
	#ifdef DEVERMINE
		printf("\nCODE = %d\n", code);
	#endif
	if(!code)
	{
		if(fprintf(dialogue,corps)<0) return -1;
		if(fprintf(dialogue,".\r\n")<0) return -1;
		//dialogue_generique(dialogue, NULL, NULL, NULL, erreur, taille);
		return retour_generique(dialogue,SUCCES_DATA_CODE,erreur,taille);
	}
	else
		return -1;
}

int SMTP_dialogue(FILE *dialogue,struct courriel *courriel,char *erreur,int taille){
if(dialogue_connexion(dialogue,erreur,taille)<0) return -1;
if(dialogue_EHLO(dialogue,courriel->hello,erreur,taille)<0) return -1;
if(dialogue_MAIL(dialogue,courriel->expediteur,erreur,taille)<0) return -1;
if(dialogue_RCPT(dialogue,courriel->destinataire,erreur,taille)<0) return -1;
if(dialogue_DATA(dialogue,courriel->corps,erreur,taille)<0) return -1;
if(dialogue_QUIT(dialogue,erreur,taille)<0) return -1;
return 0;
}

#ifdef DEVERMINE
// Fonction d'affichage de message

void afficheCourriel(FILE *sortie,struct courriel *donnees,unsigned char complet){
fprintf(sortie,"COURRIEL");
if(donnees->hello[0]!='\0')
  fprintf(sortie," signature %s\n",donnees->hello);
else
  fprintf(sortie,"\n");
if(donnees->expediteur[0]!='\0' && donnees->destinataire[0]!='\0')
  fprintf(sortie,"  %s -> %s\n",donnees->expediteur,donnees->destinataire);
if(donnees->corps!=NULL && donnees->taille>=0){
  fprintf(sortie,"Corps taille %d octet(s)\n",donnees->taille);
  if(complet) fprintf(sortie,donnees->corps);
  else{
    int c=0,cpt=0;
    while(donnees->corps[c]!='\0'){ if(donnees->corps[c]=='\n') cpt++; c++; }
    int cpt2=0;
    c=0;
    while(donnees->corps[c]!='\0'){
      if(cpt2<AFFICHE_COURRIEL_DEBUT || cpt2>cpt-AFFICHE_COURRIEL_FIN) fwrite(donnees->corps+c,sizeof(char),1,sortie);
      if(cpt2==AFFICHE_COURRIEL_DEBUT && cpt2<=cpt-AFFICHE_COURRIEL_FIN) fprintf(sortie,"...\n");
      if(donnees->corps[c]=='\n') cpt2++;
      c++;
      }
    }
  }
}
#endif
