/**
 * Taille du buffer
 */
#define BUFELEM 4
#define BUFSIZE (4 * sizeof(int))

/**
 * Nombre de clients dans le chat
 */
#define NB_CHATMAX 100

/**
 * Prix des places par catégorie
 */
int prixPlaces[3];

/**
 * Affichage des données du buffer
 */
void dumpBuffer(int * buf);

/**
 * Calcul du coût final d'une commande
 */
float howMuch(int nbPlaces, int cat, int nbEtudiant);