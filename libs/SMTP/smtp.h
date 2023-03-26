
// Constantes

#define	SMTP_PORT_DEFAULT	"25"
#define	SMTP_LOCAL_INTERFACE	"lo"

#define MAX_ADRESSE_COURRIEL	1024
#define MAX_HELLO		1024

// Structures

struct courriel {
  char hello[MAX_HELLO];
  char expediteur[MAX_ADRESSE_COURRIEL]; 
  char destinataire[MAX_ADRESSE_COURRIEL]; 
  int taille;
  char *corps;
};

// Prototypes

struct courriel *SMTP_gestion(FILE *dialogue,void (*traitement)(struct courriel *,void *),void *tparam);
int SMTP_dialogue(FILE *dialogue,struct courriel *courriel,char *erreur,int taille);
#ifdef DEVERMINE
void afficheCourriel(FILE *sortie,struct courriel *donnees,unsigned char complet);
#endif

