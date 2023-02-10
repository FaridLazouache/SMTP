#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "args.h"

// Analyse les arguments de l'exécutable

void afficheSyntaxe(char *executable){
fprintf(stderr,"Syntaxe : %s [options]\n",executable);
fprintf(stderr,"  -l|--local # lier à l'interface lo\n");
fprintf(stderr,"  -p|--port <no port> # lier à un port spécifique\n");
fprintf(stderr,"  -j|--journal <fichier> # écrire des information dans un fichier\n");
fprintf(stderr,"  -n|--niveau <0-255> # détail des informations\n");
exit(EXIT_FAILURE);
}

int analyseArguments(int argc,char *argv[],struct smtp_config *config){
static struct option long_options[] = {
  {"local",no_argument,0,'l'},
  {"port",required_argument,0,'p'},
  {"journal",required_argument,0,'j'},
  {"niveau",required_argument,0,'n'},
  {0,0,0,0}
  };
memset(config,0,sizeof(struct smtp_config));
while(1){
  int c=getopt_long(argc,argv,"lp:j:n:",long_options,NULL);
  if(c<0) break;
  switch(c){
    case 'l':
      config->local=1;
      break;
    case 'p':
      strncpy(config->port,optarg,sizeof(config->port)-1);
      break;
    case 'j':
      strncpy(config->journal,optarg,sizeof(config->journal)-1);
      break;
    case 'n':
      config->niveau=atoi(optarg);
      break;
    default:
      afficheSyntaxe(argv[0]);
      break;
    }
  }
if(optind<argc) afficheSyntaxe(argv[0]);
return 0;
}
