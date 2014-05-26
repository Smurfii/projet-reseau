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


/* 
	La fonction handler permet de libérer les processus fils si ils sont dans un état zombie.
	La fonction waitpid retourne le pid du processus libéré, et -1 s'il n'y a aucun processus à libérer ou en cas d'erreur.
	L'option WNOHANG permet de rendre la fonction non-bloquante s'il n'y a plus de fils à libérer.
*/

void handler(int sig) {

  while(waitpid(-1, NULL, WNOHANG)>0);

}
