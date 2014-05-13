#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TAILLE_MAX 80

int main(int argc, char *argv[])
{
  if (argc != 2){ perror("probleme argument");}
  FILE* fichier = NULL;
  char chaine[TAILLE_MAX] = "";
  
  fichier = fopen(argv[1], "r");
  if (fichier != NULL)
    {
      printf("\n");
      
      while (fgets(chaine, TAILLE_MAX, fichier) != NULL) /*On lit maximum TAILLE_MAX caractères du fichier, on stocke le tout dans "chaine"*/
      {
      printf("%s", chaine); /*on affiche la chaine*/
      }
      printf("\n");
      
      fclose(fichier);
    }
    return 0;
}
      
