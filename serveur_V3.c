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
/* Taille maximum des lignes dans un fichier de sauvegarde sur le serveur */
#define TAILLE_MAX 80

/* La fonction permet d'afficher le contenu d'un fichier */
void afficher(FILE* fichier){
  
  /*Initialisation des variables et du curseur */
  int position_curseur = ftell(fichier);
  char chaine[TAILLE_MAX] = "";
  
  fseek(fichier, 0, SEEK_SET);
  
  if (fichier != NULL) {
      printf("\n");
      
      /*On lit maximum TAILLE_MAX caractères du fichier, on stocke le tout dans "chaine"*/
      while (fgets(chaine, TAILLE_MAX, fichier) != NULL) {
      printf("%s", chaine); /*on affiche la chaine*/
      }
      printf("\n");
    }
    printf("\n");

    fseek(fichier, position_curseur, SEEK_SET);
}


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
	char chaine[TAILLE_MAX];
	int ret, h, nboct, i = 0, position_curseur = 0, indice = 0, j = 0;
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
		
				}
			}	 	

			fclose(fd);
			close(socket_dialogue); 			
		}	
	}	
	printf("\n");

	return 0;
}
