

Principe:

Chaque client travaillant sur le même fichier, possèdent une copie du fichier en question.
Le serveur possède lui le fichier original.
A chaque modification, l'ensemble des copies du fichier et le fichier coté serveur sont modifiés. 

****************************************************************************************************************************
Dans l'ordre, les étapes d'utilisation :

Le client se connecte au serveur :
  _ Il télécharge une copie du fichier "fichier".txt qui l'intéresse.
  
  _le texte est affiché avec le curseur du client en case 0 (au début du texte), les autres curseurs des clients connectés sont affichés à leurs positions respectives au moment même. Sachant que pendant le temps de téléchargement du fichier, les autres clients peuvent avoir modifié le fichier txt. Il y a un risque de décalage avec le fichier présenté au client et le fichier modifié. Pour cela, on peut essayer de mettre en place un "freeze" le temps que le nouveau client se connecte et télécharge sa copie. 
    
Le client modifie un fichier :  
  _Le client veut modifier le fichier. Il envoit donc sa modification: cela peut être l'ordre de déplacer le curseur, supprimer un caractère ou bien ajouter un caractère. L'ordre est envoyé par le client au serveur qui l'envoit aux autres clients. Avec cette modification le client envoit la position de son pointeur dans le texte.

Le client veut créer un fichier :
  _Il envoit un ordre de création au serveur qui va créer le fichier vide .txt. Le serveur c
  
