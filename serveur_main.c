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
#include "fonctions_serveur.h"


/* Taille du buffer permettant de stocker et retransmettre les données */
#define BUF_LEN 65536			
/* Nombre maximale de personnes pouvant se connecter au serveur en même temps */
#define MAX_PEOPLE_WAIT 5
/* Taille maximum des lignes dans un fichier de sauvegarde sur le serveur */
#define TAILLE_MAX 80

int main(int argc, char ** argv){

	/* Argument du programme */
	int port;	
	
	/* Déclaration des variables pour les sockets */
	int sock, socket_dialogue;
	struct sockaddr_in serveur, client;
	socklen_t serveur_len, client_len;	

	/*Variables de stockage et autres */
	char * buffer;
	char chaine[TAILLE_MAX];
	int ret, h, nboct, i = 0, position_curseur, indice = 0, j = 0;
	int chaine_size = 0;
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
		La "struct sigaction" permet de gérer le signal SIGCHLD, envoyé au processus père par le processus fils
		quand il passe à l'état de zombie.
		sa_handler est un pointeur vers la fonction qui sera executée quand le père recevera un signal SIGCHLD.
		Quand le père reçoit SIGCHLD, il stoppe le programme et lance la fonction handler. 
		systeme pour lancer la fonction handler.
		Le tag SA_RESTART permet de relancer automatiquement l'appel système interrompu.
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

	     	printf("Fichier \"%s\" ouvert par un client\n", nom_fichier);
	     	/* On vide le tampon de sortie et on force l'écriture */
	     	fflush(stdout);

	     	/* Gestion de la sauvegarde des fichiers et du curseur */
	
			/* Crée un string dans lequel on va stocker la chaine de caractère nomdufichier_curseur*/
			char fichier_curseur [TAILLE_MAX];
			/* Copie argv[3] dans le string */
			strcpy(fichier_curseur, nom_fichier);
			/* Concaténation fichier_curseur et "_curseur" */
			char * nom_curseur = strcat(fichier_curseur, "_curseur");
			/* Ouverture du fichier contenant la valeur de la postion du curseur */
			FILE * fdc = fopen(nom_curseur, "r+");
			if(fdc == NULL) {
				perror("Erreur lors de l'ouverture du fichier du curseur");
				exit(1);
			}
			/* Récupération de la valeur */
			fscanf(fdc, "%d", &position_curseur);
			fclose(fdc); 
			/* Réouverture du fichier afin d'écraser l'ancienne valeur du curseur par la nouvelle à la fin de la saisie du texte */
			/*fdc = fopen(nom_curseur, "w+"); 
			if(fdc == NULL) {
				perror("Erreur lors de l'ouverture du fichier du curseur");
				exit(1);
			}*/

	     	/* Création du buffer pour stocker les messages transmis */ 	
			buffer = (char *)calloc(BUF_LEN, sizeof(char));	

			/* Boucle de réception du fichier */		
			while(1) 	{

				/* En fonction de ce qu'il y aura dans le fichier, le serveur n'aura pas le même comportement vis-à-vis des clients*/
								
				/*
					Si le fichier est vide, erreur pour l'instant.
					fseek permet de positionner le curseur à la fin et ftell permet de renvoyer la position actuel du curseur.
					Si la position actuelle du curseur est celle de la fin du fichier, alors le fichier est vice. 
				*/
				fseek(fd, 0, SEEK_END);
				if(ftell(fd) == 0)
				{	
					perror("Le fichier est vide");
					exit(1);
				} 
				
				/* Si le fichier n'est pas vide */				 
				else {
					
					/* Affichage de ce qu'il y a dans le fichier de sauvegarde */
					afficher(fd);

					/* On vide le buffer pour stocker les données sauvegarder dans le fichier */
					bzero(buffer, BUF_LEN);
					bzero(chaine, TAILLE_MAX);	

					/* 
						Pour mesurer la taille du fichier, on a déplacé le curseur à la fin.
						On le repositionne au début pour pouvoir lire dans le fichier. 
					*/  	
					fseek(fd, 0, SEEK_SET);
					
					/* Récupération des données contenues dans le fichier et stockage dans le buffer */
					while(fgets(chaine, BUF_LEN, fd) != '\0') {
						
						chaine_size = strlen(chaine);
						
						for (i = 0; i < chaine_size; ++i)
							{
								buffer[i+indice] = chaine[i];
							}
						
						indice = indice + chaine_size;
						bzero(chaine, TAILLE_MAX);
						chaine_size = 0;	
					}
					indice = 0;

					if(send(socket_dialogue, buffer, BUF_LEN, 0) == -1) {
						perror("Erreur lors de la retransmission du message aux clients");
						exit(1);
					}	

					/* On vide le buffer pour stocker les données envoyées par les clients */
					bzero(buffer, BUF_LEN);

					/*Réception des messages des clients */
					ret = recv(socket_dialogue, buffer, BUF_LEN, 0);
					if(ret == -1) {
						perror("Erreur lors de la réception des données provenant du client");
						close(socket_dialogue);
						exit(1);
					}

					/*Sauvegarde sur le fichier qui se trouve sur le serveur */
					while(buffer[j] != '\0') {
						ecrire(buffer[j], fd, &position_curseur);
						j = j + 1;
					}
					j = 0;
					printf("données sauvegardées dans le fichier\n");
					/*fprintf(fdc, "%d", position_curseur);	*/
				}
			}	 	

			fclose(fd);
			/*fclose(fdc); */
			close(socket_dialogue); 			
		}	
	}	
	printf("\n");

	return 0;
}
