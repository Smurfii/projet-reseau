#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

#define BUF_LEN 65536			


int lire(char *chaine, int longueur)
{	
	/*Initialisation de la position d'entrée */
    char *positionEntree = NULL;
  
    /* On lit le texte saisi au clavier */
    if (fgets(chaine, longueur, stdin) != NULL) { /* Si la saisie se fait sans erreur : */
        
        positionEntree = strchr(chaine, '\n'); /* On recherche l'Entrée" */
        
        if (positionEntree != NULL) { /* Si on a trouvé le retour à la ligne */
      
            *positionEntree = '\0'; /* On remplace ce caractère par \0 */
        }

        return 0; /* On renvoie 0 si la fonction s'est déroulée sans erreur */
    }

    else {
        return 1; /* On renvoie 1 si la fonction s'est mal déroulée */ 
    }
}

int main(int argc, char ** argv){

/* Arguments */
char * ip_serveur;
int port;
char * nom_fichier;

/* Variables pour la création de la socket TCP */

int sock;
struct sockaddr_in serveur, client;
socklen_t serveur_len, client_len;

/*Variables de stockage et autre */
char * buffer;
int x, fd, i, nboct;

/*Valeurs par défaut des tailles des clients */
serveur_len = sizeof(serveur);
client_len = sizeof(client);

/* Vérification du nombre d'arguments */
	if (argc != 4) {
		fprintf(stderr, "Usage: %s ip_serveur, nom_fichier, port\n", argv[0]);
		exit(1);
	}

/* Stockage et conversion des arguments */
ip_serveur = argv[1];
port = atoi(argv[2]);
nom_fichier = argv[3];

/* On ouvre le fichier en lecture seule*/
fd = open(nom_fichier, O_RDONLY);
if(fd == -1) {
    perror("open");
    exit(1);
}
printf("Fichier %s ouvert...\n", nom_fichier);


/* Création de la socket TCP */
if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Erreur lors de la création du socket TCP\n");
	exit(1);
}

/* Structure pour le bind */
serveur.sin_family = AF_INET;
serveur.sin_port = htons(port);
serveur.sin_addr.s_addr = inet_addr(ip_serveur);

if (connect(sock, (struct sockaddr *) &serveur, serveur_len) < 0) {
	perror("Erreur de connection au serveur");
	exit(1);
}

printf("Vous êtes connecté");

  // Fonction send : permet d'envoyer des données lors d'une connexion TCP.
  // arg 1 : fd du socket
  // arg 2 : pointeur vers le buffer de données
  // arg 3 : nombre d'octets à envoyer
  // arg 4 : flags, ici 0
  
// On envoie la longueur du nom du fichier
x = strlen(nom_fichier);
if(send(sock, &x, sizeof(int), 0) == -1){
    perror("send");
    close(sock);
    close(fd);
    exit(1);
}
printf("Debut envoi du nom de fichier...\n");
  
// Envoi du nom du fichier
nboct = send(sock, nom_fichier, x, 0);
if(nboct == -1){
    perror("send");
    close(sock);
    close(fd);
    exit(1);
}
printf("Nom fichier envoyé...\n");
  
printf("Debut envoi du fichier...\n");
  
/* Il faut allouer le buffer d'envoi de données */
buffer = (unsigned char*)calloc(BUF_LEN, sizeof(unsigned char));
  
while(1) {

	/* Initialisation du buffer */
	for(i = 0; i < BUF_LEN-1; i++) {
		buffer[i] = '\0';
	}

	buffer[BUF_LEN-1]='\n';	

	/* On lis le message entrée par le client sur la console */
	if(lire(buffer,BUF_LEN) == 1) {
		perror("Erreur lors de la lecture du message envoyée par le client");
		exit(1);
	}

	/* On envoie le message qui a été écrit par le client */
	write(sock, buffer, BUF_LEN);
}

close(sock);

return 0;
}
