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

#define BUF_LEN 65536			
#define MAX_PEOPLE_WAIT 3

int main(int argc, char ** argv){

/* arguments */
int port;

/* Déclaration des variables pour créer la socket */
int sock, socket_dialogue;
struct sockaddr_in serveur, client;
socklen_t serveur_len, client_len;

/*Variables de stockage et autre */
char buffer[BUF_LEN];
int ret = 0 , f = 0;

/*Valeurs par défaut des tailles des clients */
serveur_len = sizeof(serveur);
client_len = sizeof(client);

/* Vérification du nombre d'arguments */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(1);
	}

/* Stockage des arguments dans des variables sous la bonne forme */
port = atoi(argv[1]);

/* Création de la socket TCP */
if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Erreur lors de la création de la socket TCP\n");
	exit(1);
}

/* Structure pour le bind */
serveur.sin_family = AF_INET;
serveur.sin_port = htons(port);
serveur.sin_addr.s_addr = INADDR_ANY;

/* Bind de la socket TCP */
if (bind(sock, (struct sockaddr *)&serveur, serveur_len) < 0) {
	perror("Erreur lors du bind de la socket TCP\n");
	exit(1);
}

/* Mise sur écoute du serveur */
if (listen(sock, MAX_PEOPLE_WAIT) == -1) {
	perror("Erreur listen");
	exit(1);
}

/* Accepte une connexion client et stock le client accepté dans client*/
if ((socket_dialogue = accept(sock, (struct sockaddr *) &client, &client_len)) < 0) {
	perror("Erreur accept");
	exit(1);
}

while ((ret = read(socket_dialogue, buffer, sizeof buffer)) > 0) {

	/* écriture de ce que l'on vient de recevoir dans le fichier f */
	if (write(f, buffer, ret) < 1) {
		perror("Erreur d'écriture");
		exit(1);
	}

	/* Pas obligatoire, surtout utile pour du débogage */
	if (ret == -1) {
		perror("Erreur read");
	}
}

close(f); 
close(sock); /* fermeture de la socket */

return 0;
}


