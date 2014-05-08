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
/* Nombre maximale de personnes pouvant se connecter au serveur en même temps */
#define MAX_PEOPLE_WAIT 5

int main(int argc, char ** argv){

/* arguments */
int port;

/* Déclaration des variables pour les sockets */
int sock, socket_dialogue;
struct sockaddr_in serveur, client;
socklen_t serveur_len, client_len;

/*Variables de stockage et autre */
unsigned char * buffer;
int ret, f = 0, h;
/* char * file_name; */

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
	perror("Erreur lors de la création du socket TCP\n");
	exit(1);
}

/* Structure pour le bind */
serveur.sin_family = AF_INET;
serveur.sin_port = htons(port);
serveur.sin_addr.s_addr = INADDR_ANY;

/* Bind de la socket TCP */
if (bind(sock, (struct sockaddr *)&serveur, serveur_len) < 0) {
	perror("Erreur lors du bind du socket TCP\n");
	exit(1);
}
printf("Socket lié au port %d...\n", port);

/* Mise sur écoute du serveur */
if(listen(sock, MAX_PEOPLE_WAIT) == -1) {
	perror("Erreur listen");
	exit(1);
}
printf("Le socket %d est maintenant en mode écoute...\n", sock);


/* Accepte une connexion client et stock le client accepté dans client*/

while(1) {
	printf("En attente de la tentative de connexion d'un client sur le port %d...\n", port);

	if((socket_dialogue = accept(sock, (struct sockaddr *) &client, &client_len)) < 0) {
		perror("Erreur accept");
		exit(1);
	}
	
	printf("Nouveau client, connecte au socket %d depuis %s:%d...\n", socket_dialogue, inet_ntoa(client.sin_addr), htons(client.sin_port));

	h = fork();

	/* Cas d'erreur */
	if(h < 0) {
		perror("Erreur lors du fork, fermeture du serveur ...");
		close(sock);
		exit(1);
	}	

	/* Cas du Père */
	if(h > 0) {
		/* On ferme le socket client pour ce processus, on en a plus besoin */
      	close(socket_dialogue);
      	/* Permet d'aller directement à la fin du corps de la boucle */
      	continue;				
	}

	/* Cas du fils */
	else {
		/* On a plus besoin du socket en mode d'écoute */
		close(sock);

		/* Partie de sauvegarde des données sur un fichier en construction */

		/* Réception des données en provenance du client */
		// Fonction recv : permet de recevoir des données du socket de
      	//                 l'arg 1
      	// arg 1 : socket renvoyée par la fonction accept, ici "csfd"
      	// arg 2 : pointeur vers le buffer dans lequel les données transmises
      	//         seront écrites
      	// arg 3 : nombre mamximum d'octets à lire depuis la socket
      	// arg 4 : flags (voir la page man pour plus de détails). 
      	//         Nous mettrons les flags à 0 (aucun flag)
      	//         Rappel sur l'utilisation des flags : 
      	//             Il faut utiliser l'opérateur '|' entre les flags pour
      	//             pouvoir les combiner

      	// Cette première utilisation de recv va nous permettre de récupérer 
      	// la taille du nom du fichier, ensuite nous recevrons le nom
		/*if(recv(socket_dialogue, &nboct, sizeof(int), 0) == -1){
			perror("Erreur de réception des données provenant du client");
			close(socket_dialogue);
			exit(1);
      	}

      	file_name = calloc(nboct + 1, sizeof(char));
      
      	nboct = recv(socket_dialogue, file_name, nboct, 0);
      	
      	if(nboct == -1){
			perror("Erreur de réception des données provenant du client");
			close(socket_dialogue);
			exit(1);
     	}
		
		printf("Création du fichier : %s...\n", file_name);

		file_name = calloc()

			fd = open(file_name, O_WRONLY | O_CREAT);
			if(fd == -1) {
				perror("Erreur lors de la création du fichier de stockage sur le serveur");
				close(socket_dialogue);
				exit(1);
     	 	}

     	 	printf("Fichier \"%s\" ouvert...\n", file_name);*/



     	/* Création du buffer pour stocker les messages envoyer */ 	
		buffer = (unsigned char*)calloc(BUF_LEN, sizeof(unsigned char));

		/* Boucle de réception du fichier */
		do {
			/* On vide le buffer */
			bzero(buffer, BUF_LEN);

			ret = read(socket_dialogue, buffer, BUF_LEN);
			if(ret == -1) {
				perror("Erreur lors de la réception des données provenant du client");
				close(socket_dialogue);
				exit(1);
			}

			if((write(f, buffer, ret)) == -1) {
				perror("Erreur d'écriture des données dans le fichier de sauvegarde");
				close(socket_dialogue);
				exit(1);
			}

		} while(ret == BUF_LEN);

		close(f); 
		close(socket_dialogue); 
		
		return 0;
	}	
}

printf("\n");

return 0;
}

