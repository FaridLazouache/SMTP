
// Prototypes

int initialisationServeur(char *service,int connexions,char *interface);
int boucleServeur(int ecoute,int (*traitement)(int));
int connexionServeur(char *hote,char *service);

char **chercherMX(char *domaine);
