// Constantes

#define CONFIG_MAX_PORT		16
#define CONFIG_MAX_JOURNAL	1024

// Structure pour la configuration

struct smtp_config{
  char journal[CONFIG_MAX_JOURNAL];  
  int niveau;
  char port[CONFIG_MAX_PORT];  
  unsigned char local;
};

// Prototype

int analyseArguments(int argc,char *argv[],struct smtp_config *config);
