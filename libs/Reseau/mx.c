#include <stdlib.h>
#include <string.h>
#include <resolv.h>

#include "reseau.h"

// Constantes locales

#define MAX_TAMPON	4096

// Fonctions de résolution

struct _couple{
  int priorite;
  char *nom;
  };

static int _couplecmp(const void *p1,const void *p2){
const struct _couple *c1=p1,*c2=p2;
return c1->priorite-c2->priorite;
}

char **chercherMX(char *domaine){
unsigned char tampon[MAX_TAMPON];
char texte[MAX_TAMPON];
ns_msg msg={0};
ns_rr rr;
struct __res_state etat={0};
int statut=res_ninit(&etat);
if(statut){
  perror("chercherMX.res_ninit");
  exit(EXIT_FAILURE);
  }
memset(tampon,0,sizeof(tampon));
int taille=res_nquery(&etat,domaine,C_IN,T_MX,tampon,sizeof(tampon)-1);
if(taille==0){ res_nclose(&etat); return NULL; }
statut=ns_initparse(tampon,taille,&msg);
if(statut<0){ res_nclose(&etat); return NULL; }
int nombre=ns_msg_count(msg,ns_s_an);
struct _couple *lesmx=calloc(nombre,sizeof(struct _couple));
int i;
for(i=0;i<nombre;i++){
  statut=ns_parserr(&msg,ns_s_an,i,&rr);
  if(statut<0) break;
  int type=ns_rr_type(rr);
  if(type==ns_t_mx){
    unsigned char *valeur=(unsigned char *)ns_rr_rdata(rr);
    int priorite;
    NS_GET16(priorite,valeur);
    int statut=dn_expand(ns_msg_base(msg),ns_msg_end(msg),valeur,texte,MAX_TAMPON);
    if(statut<0) strcpy(texte,"");
    lesmx[i].priorite=priorite;
    lesmx[i].nom=malloc(strlen(texte)+1);
    strcpy(lesmx[i].nom,texte);
#ifdef DEVERMINE
    printf("#%d [%d/%s]\n",i,priorite,texte);
#endif
    }
  }
res_nclose(&etat);
if(nombre<=0) return NULL;
qsort(lesmx,nombre,sizeof(struct _couple),_couplecmp);
char **resultat=calloc(nombre+1,sizeof(char *));
for(i=0;i<nombre;i++) resultat[i]=lesmx[i].nom;
free(lesmx);
return resultat;
}
