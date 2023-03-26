// Constantes

#define CONFIG_MAX_PORT		16
#define CONFIG_MAX_JOURNAL	1024
#define CONFIG_MAX_CHEMIN	1024
#define CONFIG_MAX_INTERFACE	1024

// Structure pour la configuration

struct smtp_config{
  char journal[CONFIG_MAX_JOURNAL];  
  int niveau;
  char port[CONFIG_MAX_PORT];  
  char dossier[CONFIG_MAX_CHEMIN];  
  char carte[CONFIG_MAX_CHEMIN];  
  unsigned char local;
  char interface[CONFIG_MAX_INTERFACE]
};

// Prototype

int analyseArguments(int argc,char *argv[],struct smtp_config *config);
