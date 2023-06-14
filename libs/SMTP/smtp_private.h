// Constantes

#define ACCUEIL_CODE		220
#define ACCUEIL_TEXTE		"Welcome"

#define ERREUR_SYNTAXE		"Syntax error"

#define ERREUR_HELO_CODE	502
#define ERREUR_HELO_TEXTE	" - correct syntax is \"EHLO username\""
#define ERREUR_MAIL_CODE	501
#define ERREUR_MAIL_TEXTE	ERREUR_SYNTAXE
#define ERRORD_MAIL_CODE	503
#define ERRORD_MAIL_TEXTE	"Need EHLO first"
#define ERREUR_RCPT_CODE	501
#define ERREUR_RCPT_TEXTE	ERREUR_SYNTAXE
#define ERRORD_RCPT_CODE	503
#define ERRORD_RCPT_TEXTE	"Need EHLO first"
#define ERREUR_DATA_CODE	501
#define ERREUR_DATA_TEXTE	ERREUR_SYNTAXE
#define ERRORD_DATA_CODE	503
#define ERRORD_DATA_TEXTE	"Need MAIL and RCPT first"
#define ERREUR_QUIT_CODE	501
#define ERREUR_QUIT_TEXTE	ERREUR_SYNTAXE

#define SUCCES_HELO_CODE	250
#define SUCCES_HELO_TEXTE	"Hi!"
#define SUCCES_MAIL_CODE	250
#define SUCCES_MAIL_TEXTE	"Sender recorded"
#define SUCCES_RCPT_CODE	250
#define SUCCES_RCPT_TEXTE	"Target recorded"
#define SUCCES_DATA_CODE	250
#define SUCCES_DATA_TEXTE	"Accepted"
#define SUCCES_QUIT_CODE	221
#define SUCCES_QUIT_TEXTE	"Bye!"

#define CONTINUE_DATA_CODE	354
#define CONTINUE_DATA_TEXTE	"Type email, end with ."

#define MAIL_TAG		"FROM:"
#define RCPT_TAG		"TO:"

#define DATA_BLOCK		2048
