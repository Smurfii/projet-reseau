#include <gtk/gtk.h>
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

#define BUF_LEN 65536     
#define TAILLE_MAX 80

/* Lire retourne 0 si elle s'est bien déroulée, 1 sinon
   Elle lit les caractères entrées par l'utilisateur dans le terminal*/
int lire(char *chaine, int longueur) {   
  
  /* On lit le texte saisi au clavier */
  if (fgets(chaine, longueur, stdin) != NULL) { 
        
      return 0; /* On renvoie 0 si la fonction s'est déroulée sans erreur */
  }

  else {
      return 1; /* On renvoie 1 si la fonction s'est mal déroulée */ 
  }
}

int main(int argc, char ** argv){

  GtkWidget* pWindow;
  GtkWidget* pLabel;
 
  gtk_init(&argc,&argv);
 
  pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(pWindow), "Editeur de texte collaboratif");
  gtk_window_set_default_size(GTK_WINDOW(pWindow), 320, 200);

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
  int x, nboct = 0, ret = 0, mes_len = 0;
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

  /* On ouvre le fichier en lecture seule*/
  fd = fopen(nom_fichier, "r");
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
      /* Creation du label */
      pLabel=gtk_label_new(buffer);

      /* On ajoute le label a l'intérieur de la fenêtre */
      gtk_container_add(GTK_CONTAINER(pWindow), pLabel);
 
      /* Affichage de la fenêtré et de tout ce qu'il contient */
      gtk_widget_show_all(pWindow);
 
      /* Connexion du signal
      /* On appelle directement la fonction de sortie de boucle */
      g_signal_connect(G_OBJECT(pWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);
 
    printf("%s", buffer);
    printf("\n");
      
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

  gtk_main();

  /* Fermeture de la socket */  
  close(sock);  

  return 0;
}


