#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <netinet/tcp.h>
#include <net/if.h>

#include "reseau.h"

// Fonctions pour le mode connexion

int initialisationServeur(char *service,int connexions,char *interface){
struct addrinfo precisions,*resultat,*origine;
int statut;
int s;

/* Construction de la structure adresse */
memset(&precisions,0,sizeof precisions);
precisions.ai_family=AF_UNSPEC;
precisions.ai_socktype=SOCK_STREAM;
precisions.ai_flags=AI_PASSIVE;
statut=getaddrinfo(NULL,service,&precisions,&origine);
if(statut<0){ perror("initialisationServeur.getaddrinfo"); exit(EXIT_FAILURE); }
struct addrinfo *p;
for(p=origine,resultat=origine;p!=NULL;p=p->ai_next)
  if(p->ai_family==AF_INET6){ resultat=p; break; }

/* Creation d'une socket */
s=socket(resultat->ai_family,resultat->ai_socktype,resultat->ai_protocol);
if(s<0){ perror("initialisationServeur.socket"); exit(EXIT_FAILURE); }

/* Options utiles */
int vrai=1;
if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&vrai,sizeof(vrai))<0){
  perror("initialisationServeur.setsockopt (REUSEADDR)");
  exit(EXIT_FAILURE);
  }
if(setsockopt(s,IPPROTO_TCP,TCP_NODELAY,&vrai,sizeof(vrai))<0){
  perror("initialisationServeur.setsockopt (NODELAY)");
  exit(EXIT_FAILURE);
  }
if(interface!=NULL){
  struct ifreq interf;
  memset(&interf,0,sizeof(interf));
  strncpy(interf.ifr_name,interface,sizeof(interf.ifr_name));
  if(setsockopt(s,SOL_SOCKET,SO_BINDTODEVICE,(void *)&interf,sizeof(interf))< 0){
    perror("initialisationServeur.setsockopt (BINDTODEVICE)");
    exit(EXIT_FAILURE);
    }
  }

/* Specification de l'adresse de la socket */
statut=bind(s,resultat->ai_addr,resultat->ai_addrlen);
if(statut<0){ freeaddrinfo(origine); shutdown(s,SHUT_RDWR); return -1; }

/* Liberation de la structure d'informations */
freeaddrinfo(origine);

/* Taille de la queue d'attente */
statut=listen(s,connexions);
if(statut<0){ shutdown(s,SHUT_RDWR); return -2; }

return s;
}

int boucleServeur(int ecoute,int (*traitement)(int)){
int dialogue;
while(1){
    /* Attente d'une connexion */
    if((dialogue=accept(ecoute,NULL,NULL))<0) return -1;

    /* Passage de la socket de dialogue a la fonction de traitement */
    if(traitement(dialogue)<0){ shutdown(ecoute,SHUT_RDWR); return 0;}
    }
}

int connexionServeur(char *hote,char *service){
struct addrinfo precisions,*origine;
int statut;
int s;

/* Creation de l'adresse de socket */
memset(&precisions,0,sizeof precisions);
precisions.ai_family=AF_UNSPEC;
precisions.ai_socktype=SOCK_STREAM;
statut=getaddrinfo(hote,service,&precisions,&origine);
if(statut<0){ perror("connexionServeur.getaddrinfo"); exit(EXIT_FAILURE); }
struct addrinfo *p=NULL;
unsigned char ipv6=0;
unsigned char ipv4=0;
while(1){
  if(ipv4==1 && ipv6==1)
    { freeaddrinfo(origine); return -1; }
  if(p==NULL) p=origine;
  if((ipv6==0 && p->ai_family==AF_INET6) || (ipv6==1 && ipv4==0 && p->ai_family==AF_INET)){
#ifdef DEBUG
    fprintf(stderr,"Connexion sur "%s" ipv6=%d ipv4=%d famille=%d\n",hote,ipv6,ipv4,p->ai_family);
#endif
    }
    /* Creation d'une socket */
    s=socket(p->ai_family,p->ai_socktype,p->ai_protocol);
    if(s<0){ perror("connexionServeur.socket"); exit(EXIT_FAILURE); }

    /* Connection de la socket a l'hote */
    if(connect(s,p->ai_addr,p->ai_addrlen)==0) break;
    }

/* Liberation de la structure d'informations */
freeaddrinfo(origine);

return s;
}
