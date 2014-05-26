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

/* Lire retourne 0 si elle s'est bien déroulée, 1 sinon
   Elle lit les caractères entrées par l'utilisateur dans le terminal*/
int lire(char *chaine, int longueur){   
	
	/* On lit le texte saisi au clavier */
	if (fgets(chaine, longueur, stdin) != NULL) { 
				
			return 0; /* On renvoie 0 si la fonction s'est déroulée sans erreur */
	}

	else {
			return 1; /* On renvoie 1 si la fonction s'est mal déroulée */ 
	}
}

void decaler(int position_curseur, FILE* fichier){
	
	/* Initialisation des variables et du curseur */
	int i = 0;
	fseek(fichier, 0, SEEK_END);
	int fin = ftell(fichier);
	
	fseek(fichier, position_curseur, SEEK_SET);
	
	int size = fin - position_curseur;
	int buf[size];
	int caractere = 0;
	
	/* Tant que le fichier est non vide, on récupère les caractères entrés et on les stocke */
	while(!feof(fichier)) {
		caractere = fgetc(fichier);
		buf[i] = caractere;
		i = i+1;
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
