#include <stdlib.h>
#include <stdio.h>
#include "common.h"

/**
 * Tickets Wizard
 * https://github.com/khannurien/tickets-wizard/
 * Vincent Lannurien <21002854>
 * 
 * UBO Brest, 2019
 * GNU GPL v3
 */

/**
 * Affichage des données du buffer
 */
void dumpBuffer(int * buf) {
	int i;
	for (i = 0; i < BUFELEM; i++) {
		printf("buf[%d] = %d\n", i, (int) buf[i]);
	}
}

/**
 * Calcul du coût final d'une commande
 */
float howMuch(int nbPlaces, int cat, int nbEtudiant) {
	float result = 0;
	int prixPlace;

	switch (cat) {
		case 1:
			prixPlace = prixPlaces[0];
			break;
		case 2:
			prixPlace = prixPlaces[1];
			break;
		case 3:
			prixPlace = prixPlaces[2];
			break;
		default:
			perror("howMuch");
			exit(EXIT_FAILURE);
	}

	if (nbEtudiant > 0) {
		int i;
		for (i = 0; i < nbPlaces; i++) {
			if (nbEtudiant > 0) {
				result += prixPlace * 0.8;
				nbEtudiant--;
			} else {
				result += prixPlace;
			}
		}
	} else {
		result = nbPlaces * prixPlace;
	}

	return result;
}
