#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#define TAILLE_MAX 80

// fonction qui concatène deux entiers : intcat(42, 666) renverra 42666
int intcat(int v1, int v2){
    /* On récupère le nombre de chiffres du premier nombre */
    int n = floor(log10(v1) + 1);
    /* On mulitplie le premier nombre par 10^n et on additionne le second */
    int result = v1 * pow(10, n) + v2;
    return result;
}

// fonction qui décale tous les caractères d'un fichier de 1 à partir de la position du curseur et écrit "_" à la position initiale du curseur
void decaler(int position_curseur, FILE* fichier){
	int i=0;
	fseek(fichier, 0, SEEK_END);
	int fin=ftell(fichier);
	fseek(fichier, position_curseur, SEEK_SET);
	int size = fin - position_curseur;
	int buf[size];
	int caractere = 0;
	while(!feof(fichier))
	{
		caractere = fgetc(fichier);
		printf("%d", caractere);
		buf[i] = caractere;
		i=i+1;
	}
	fseek(fichier, position_curseur, SEEK_SET);
	fputc('_', fichier);
	for(i=0; i<size; i=i+1)
	{
		fputc(buf[i], fichier);
	}
	fseek(fichier, position_curseur, SEEK_SET);
}

//fonction qui affiche ce qu'il y a d'écrit dans un fichier
void afficher(FILE* fichier){
  int position_curseur = ftell(fichier);
  char chaine[TAILLE_MAX] = "";
  fseek(fichier, 0, SEEK_SET);
  if (fichier != NULL)
    {
      printf("\n");
      
      while (fgets(chaine, TAILLE_MAX, fichier) != NULL) /*On lit maximum TAILLE_MAX caractères du fichier, on stocke le tout dans "chaine"*/
      {
      printf("%s", chaine); /*on affiche la chaine*/
      }
      printf("\n");
      
    }
    printf("\n");
    fseek(fichier, position_curseur, SEEK_SET);
}

//fonction qui écrit une lettre dans un fichier
void ecrire(char lettre, FILE* fichier, int *position_curseur){
	decaler(*position_curseur, fichier);
	fputc(lettre, fichier);
	*position_curseur = *position_curseur + 1;
}

void supprimer(FILE* fichier, int *position_curseur)
{

}

void coucou(char* fichier){
	int position_curseur;
	char nom_fichier[TAILLE_MAX];
	strcpy(nom_fichier, fichier);
	char* nom_curseur = strcat(nom_fichier, "_curseur");
	FILE* fdc = fopen(nom_curseur, "r+");
	FILE* fd = fopen(fichier, "r+");
	fscanf(fdc, "%d", &position_curseur);
	fclose(fdc); //on ferme le fichier
	fdc = fopen(nom_curseur, "w+"); //on réouvre le fichier en écrasant l'ancienne position du curseur.
	printf("%d", position_curseur);
	char lettre;
	if(fd==NULL)
	{
		perror("fopen_fichier");
	}
	afficher(fd);
	printf("écrivez votre texte\n");
	while(lettre!='$')
	{
		if(lettre=='$')
			{break;}
		lettre = getchar();
		ecrire(lettre, fd, &position_curseur);
	}
	fclose(fd);
	fprintf(fdc, "%d", position_curseur);
}

int main(int argc, char **argv)
{
	coucou(argv[1]);
    if(argc != 2)
    {   
        perror("mauvais nombre d'arguments");
        return 1;
    }
	return 0;
}
