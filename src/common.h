/**
 * Taille du buffer
 */
#define BUFELEM 4
#define BUFSIZE (4 * sizeof(int))

/**
 * Affichage des données du buffer
 */
void dumpBuffer(int * buf);

/**
 * Calcul du coût final d'une commande
 */
int howMuch(int nbPlaces, int cat, int nbEtudiant);