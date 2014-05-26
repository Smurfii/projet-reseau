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
#include "fonctions_client.h"

#define BUF_LEN 65536			
#define TAILLE_MAX 80


int main(int argc, char ** argv){

	/* Arguments */
	char * ip_serveur;
	int port;
	char * nom_fichier; 

	/* Variables pour la création de la socket TCP */ 
	int sock;
	struct sockaddr_in serveur;
	socklen_t serveur_len;

	/*Variables de stockage et autres */
	char * buffer;
	int x, nboct = 0, ret = 0, mes_len = 0, j = 0, position_curseur;
	FILE * fd = NULL; 


	/*Valeurs par défaut des tailles des structures*/
	serveur_len = sizeof(serveur);

	/* Vérification du nombre d'arguments */
		if (argc != 4) {
			fprintf(stderr, "Usage: %s ip_serveur, nom_fichier, port\n", argv[0]);
			exit(1);
		}  

	/* Stockage et conversion des arguments */
	ip_serveur = argv[1];
	port = atoi(argv[2]);
	nom_fichier = argv[3];  

	/* Gestion de la sauvegarde des fichiers et du curseur */
	
	/* Crée un string dans lequel on va stocker la chaine de caractère nomdufichier_curseur*/
	char fichier_curseur [TAILLE_MAX];
	/* Copie argv[3] dans le string */
	strcpy(fichier_curseur, argv[3]);
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

	/* On ouvre le fichier en lecture seule*/
	fd = fopen(nom_fichier, "r+");
	if(fd == NULL) {
			perror("Impossible d'ouvrir le fichier spécifié");
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

	/* Tentative de connection au serveur*/
	if (connect(sock, (struct sockaddr *) &serveur, serveur_len) < 0) {
		perror("Erreur de connection au serveur");
		exit(1);
	} 

	printf("Vous êtes connecté"); 

		/* 
			La fonction send permet d'envoyer des données au socket TCP connecté.
			arg 1 : Le file descriptor du socket
			arg 2 : Le pointeur vers le buffer permettant de stocker les données
			arg 3 : Le nombre d'octet à envoyer
			arg 4 : Le flags que l'on fixe à 0 
		*/
		
	/*On envoie la longueur du nom du fichier au serveur*/
	x = strlen(nom_fichier);
	if(send(sock, &x, sizeof(int), 0) == -1){
			perror("send");
			close(sock);
			fclose(fd);
			exit(1);
	}
	printf("Début envoi du nom de fichier...\n");
		
	/*Envoi du nom du fichier au serveur*/
	nboct = send(sock, nom_fichier, x, 0);
	if(nboct == -1){
			perror("send");
			close(sock);
			fclose(fd);
			exit(1);
	} 		
	printf("Envoi du fichier au serveur ...\n");
		
	/*Allocation du buffer permettant de stocker les données qui vont être envoyée */
	buffer = (char *)calloc(BUF_LEN, sizeof(char));
		
	/* Début de la boucle d'envoi et réception */	
	while(1) {  

		/* Initialisation du buffer pour recevoir des données */
		bzero(buffer, BUF_LEN);

		printf("Le fichier %s contient :\n\n", argv[3]);
		/* Tant que le serveur n'a pas transmis toutes les données issues du fichier de sauvegarde */	
		if(recv(sock, buffer, BUF_LEN, 0) == -1) {
			perror("Erreur de réception lors de la transmission des données\n");
			exit(1);					
		}
		printf("%s", buffer);
		printf("\n");
		
		/*Sauvegarde sur le fichier qui se trouve sur le serveur */
		while(buffer[j] != '\0') {
			ecrire(buffer[j], fd, &position_curseur);
			j = j + 1;
		}
		j = 0;
		printf("données sauvegardées dans le fichier\n");
		/*fprintf(fdc, "%d", position_curseur);*/

		/* Initialisation du buffer pour l'envoi de données */
		bzero(buffer, BUF_LEN);

		printf("Vous pouvez modifier le fichier %s\n", argv[3]);

		/* On lit les données entrées par l'utilisateur dans le terminal */
		if((lire(buffer,BUF_LEN)) == 1) {
			perror("Erreur lors de la lecture du message envoyée par l'utilisateur");
			exit(1);
		} 
		
		/* Envoi au serveur le message envoyé par la console */		
		if((send(sock, buffer, BUF_LEN, 0) == -1)) {
			perror("Erreur d'envoi de données au serveur");
			exit(1);
		}	

	}/* Fin de la boucle d'envoi et réception */ 

	/* Fermeture de la socket */	
	close(sock);
	/*fclose(fdc);*/
	fclose(fd);  

	return 0;
}
	
