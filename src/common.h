/**
 * Tickets Wizard
 * https://github.com/khannurien/tickets-wizard/
 * Vincent Lannurien <21002854>
 * 
 * UBO Brest, 2019
 * GNU GPL v3
 */

/**
 * Nombre de places
 */
#define MAXPLACES 300
#define MAXCAT1 50
#define MAXCAT2 150
#define MAXCAT3 100

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
