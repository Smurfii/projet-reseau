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

#define BUF_LEN 256			

int main(int argc, char ** argv){

/* Arguments */
char * ip_serveur, * nom_fichier;
int port;

/* Variables pour la création de la socket TCP */

int sock;
struct sockaddr_in serveur, client;
socklen_t serveur_len, client_len;

/*Variables de stockage et autre */
char buffer[BUF_LEN];
int ret, f, i;

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

/* Création de la socket TCP */
if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Erreur lors de la création de la socket TCP\n");
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

/* Initialisation du buffer */
for(i = 0; i < BUF_LEN; i++)
{
	buffer[i] = '\0';
}

scanf("%s", buffer);
write(sock, buffer, BUF_LEN);


/*
f = open(nom_fichier, O_RDONLY);

while((ret = read(f, buffer, sizeof buffer)) > 0){
	if(write(sock, buffer, ret) < 1)
	{
		perror("Erreur d'envoi au serveur");
		exit(1);
	}
}
*/

close(sock);

return 0;
}
