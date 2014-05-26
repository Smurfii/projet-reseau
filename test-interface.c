#include <stdlib.h>
#include <gtk/gtk.h>
 
int main(int argc,char **argv)
{
    GtkWidget* pWindow;
    GtkWidget* pLabel;
 
    gtk_init(&argc,&argv);
 
    pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(pWindow), "Les labels");
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 320, 200);
 
    /* Creation du label */
    pLabel=gtk_label_new("Hello World!");
 
    /* On ajoute le label a l'intérieur de la fenêtre */
    gtk_container_add(GTK_CONTAINER(pWindow), pLabel);
 
    /* Affichage de la fenêtré et de tout ce qu'il contient */
    gtk_widget_show_all(pWindow);
 
    /* Connexion du signal
    /* On appelle directement la fonction de sortie de boucle */
    g_signal_connect(G_OBJECT(pWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);
 
    gtk_main();
 
    return EXIT_SUCCESS;
}
