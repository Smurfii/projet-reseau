#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

/* Taille du buffer permettant de stocker et retransmettre les données */
#define BUF_LEN 65536			
/* Nombre maximale de personnes pouvant se connecter au serveur en même temps */
#define MAX_PEOPLE_WAIT 5

/* La fonction decaler permet de repositionner le curseur lors de l'écriture dans un fichier */
void decaler(int position_curseur, FILE* fichier){
	
	/* Initialisation des variables et du curseur */
	int i=0;
	fseek(fichier, 0, SEEK_END);
	int fin=ftell(fichier);
	
	fseek(fichier, position_curseur, SEEK_SET);
	
	int size = fin - position_curseur;
	int buf[size];
	int caractere = 0;
	
	/* Tant que le fichier est non vide, on récupère les caractères entrés et on les stocke */
	while(!feof(fichier)) {
		caractere = fgetc(fichier);
		buf[i] = caractere;
		i=i+1;
	}
	
	/* Positionnement du curseur */	
	fseek(fichier, position_curseur, SEEK_SET);
	fputc('_', fichier);

	for(i=0; i<size; i=i+1) {
		fputc(buf[i], fichier);
	}

	/* On replace le curseur pour la prochaine sauvegarde dans le fichier */
	fseek(fichier, position_curseur, SEEK_SET);
}


/* Ecrit un caractère passé en paramètre dans le fichier */
void ecrire(char lettre, FILE* fichier, int *position_curseur){
	
	/* Positionnement du curseur pour ne pas écraser les données déjà éxistante */
	decaler(*position_curseur, fichier);
	/* Sauvegarde du caractère dans le fichier */
	fputc(lettre, fichier);
	/* Repositionnement du curseur */
	*position_curseur = *position_curseur + 1;
}


/* 
	La fonction handler permet de libérer les processus fils si ils sont dans un état zombie.
	La fonction waitpid retourne le pid du processus libéré, et -1 s'il n'y a aucun processus à libérer ou en cas d'erreur.
	L'option WNOHANG permet de rendre la fonction non-bloquante s'il n'y a plus de fils à libérer.
*/

void handler(int sig) {

  while(waitpid(-1, NULL, WNOHANG)>0);

}

int main(int argc, char ** argv){

	/* Argument du programme */
	int port;	
	
	/* Déclaration des variables pour les sockets */
	int sock, socket_dialogue;
	struct sockaddr_in serveur, client;
	socklen_t serveur_len, client_len;	

	/*Variables de stockage et autres */
	char * buffer;
	int ret, h, nboct, i = 0, position_curseur = 0;
	struct sigaction act;
	char * nom_fichier = NULL; 
	FILE * fd = NULL;	

	/*Valeurs par défaut des tailles des clients */
	serveur_len = sizeof(serveur);
	client_len = sizeof(client);

	/* Vérification du nombre d'arguments */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(1);
	}
	/*
		Modification du sigaction
		On remplie la structure "struct sigaction" pour définir la gestion
		du signal SIGCHLD, envoyé au processus père par le processus fils
		quand il passe à l'état de zombie.
		sa_handler : pointeur vers la fonction qui sera executée quand le père
		recevera un signal SIGCHLD.
		Le pointeur doit être du type : void function(int)
		La fonction "sigaction" va lier un signal (ici SIGCHLD) à un traitement
		sous la forme de la structure struct sigaction (ici act)
		Le tag SA_RESTART permet de relancer les appels systÃemes interompues par le traitement du signal donnÃ© (ici SIGCHLD)
		En effet, quand le pere va recevoir un SIGCHLD, il va stoper l'appel 
		systeme pour lancer la fonction handler. Avec le tag, il relancera
		automatiquement l'appel systeme.
	*/

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_RESTART;
	act.sa_handler = handler;
	sigaction(SIGCHLD, &act, NULL);	

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

	/* 
		Mise sur écoute du serveur 
		Un maximum de 5 clients pourront demander en même temps une conenction au serveur 
	*/
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

		/* Chaque processus fils du serveur gére un client */
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

			/*
				La fonction recv permet de recevoir des données du socket TCP client.
	      		arg 1 : socket client (établit via accept)
	      	 	arg 2 : pointeur vers le buffer dans lequel les données transmises seront sauvegardées
	      	 	arg 3 : nombre maximum d'octets à lire depuis la socket
	      	 	arg 4 : flags que l'on fixe à 0
	      	*/ 		

	      	/* Récupération de la taille du nom du fichier */
			if(recv(socket_dialogue, &nboct, sizeof(int), 0) == -1){
				perror("Erreur de réception des données provenant du client");
				close(socket_dialogue);
				exit(1);
	      	}	

	      	nom_fichier = calloc(nboct + 1, sizeof(char));
	      
	      	/* Récupération du nom du fichier envoyé par le client */
	      	nboct = recv(socket_dialogue, nom_fichier, nboct, 0);
	      	if(nboct == -1){
				perror("Erreur de réception des données provenant du client");
				close(socket_dialogue);
				exit(1);
	     	}

	     	/* On place un \0 à la fin du fichier pour bien fixer sa fin */
	     	nom_fichier[nboct] = '\0';
			
			/* Création du fichier sur le serveur en mode lecture et écriture */
			printf("Création du fichier : %s...\n", nom_fichier);	

			fd = fopen(nom_fichier, "r+");
			if(fd == NULL) {
					perror("Erreur lors de la création du fichier de stockage sur le serveur");
					close(socket_dialogue);
					exit(1);
	     	 }	

	     	printf("Fichier \"%s\" ouvert...\n", nom_fichier);
	     	/* On vide le tampon de sortie et on force l'écriture */
	     	fflush(stdout);

	     	/* Création du buffer pour stocker les messages transmis */ 	
			buffer = (char *)calloc(BUF_LEN, sizeof(char));	

			/* Boucle de réception du fichier */		
			while(1) {
				
				/* On vide le buffer */
				bzero(buffer, BUF_LEN);	
				
				/*Réception des messages des clients */
				ret = recv(socket_dialogue, buffer, BUF_LEN, 0);
				if(ret == -1) {
					perror("Erreur lors de la réception des données provenant du client");
					close(socket_dialogue);
					exit(1);
				}

				/*Sauvegarde sur le fichier qi se trouve sur le serveur */
				while(buffer[i] != '\0') {
					ecrire(buffer[i], fd, &position_curseur);
					i = i + 1;
				}
				i = 0;

				/*Emission du message à tous les clients */

				
			} 	

			fclose(fd); 
			close(socket_dialogue); 
			
			return 0;
		}	
	}	
	printf("\n");

	return 0;
}

