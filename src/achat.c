#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

/**
 * Tickets Wizard
 * Vincent Lannurien
 * UBO Brest, 2019
 * GNU GPL v3
 * 
 * ACHAT
 * Application d'achat de places de concert.
 * 
 * Demande à l'utilisateur :
 *  - le nombre de places désirées ;
 *  - la catégorie souhaitée ;
 *  - le nombre de places en tarif étudiant.
 * 
 * Transmet la demande à CONCERT :
 *  - s'il reste suffisamment de places, demande à l'utilisateur un numéro de CB :
 *    - transmet le numéro à CONCERT ;
 *    - termine la transaction après validation du paiement.
 *  - s'il ne reste plus de place disponible, informe l'acheteur et termine la transation ;
 *  - s'il reste des places en nombre insuffisant, propose de reformuler une demande.
 * 
 */

int main(int argc, char * argv[]) {
	if (argc != 3) {
		printf("Usage: %s <CONCERT hostname> <CONCERT port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// boucle de demande
	while (1) {
		// identifiant de demande
		int timestamp = (int) time(NULL);
		// entrée utilisateur
		int nbPlaces;
		int cat;
		int nbEtudiant;

		// vérification de l'entrée utilisateur
		char * p, input[256];
		
		printf("Nouvelle commande. Combien de places ?\n");
		while (fgets(input, sizeof(input), stdin)) {
			nbPlaces = strtol(input, &p, 10);

			if (p == input || * p != '\n') {
				printf("Veuillez entrer une valeur entière : ");
			} else if (nbPlaces <= 0) {
				printf("Veuillez entrer une valeur supérieure à 0 : ");
			} else break;
		}

		printf("Quelle catégorie ?\n");
		while (fgets(input, sizeof(input), stdin)) {
			cat = strtol(input, &p, 10);

			if (p == input || * p != '\n') {
				printf("Veuillez entrer une valeur entière : ");
			} else if ((cat != 1) && (cat != 2) && (cat != 3)) {
				printf("Veuillez choisir une catégorie entre 1 et 3 : ");
			} else break;
		}

		printf("Combien de places en tarif étudiant ?\n");
		while (fgets(input, sizeof(input), stdin)) {
			nbEtudiant = strtol(input, &p, 10);

			if (p == input || * p != '\n') {
				printf("Veuillez entrer une valeur entière : ");
			} else if ((nbEtudiant > nbPlaces) || (nbEtudiant < 0)) {
				printf("Veuillez entrer une valeur entre 0 et le nombre de places commandées : ");
			} else break;
		}

		// connexion à CONCERT
		printf("Connexion au serveur CONCERT...\n");
		// port client
		unsigned int port;
		// socket client
		int sock;
		struct sockaddr_in addr;
		struct hostent * host;

		// buffer
		int buf[BUFELEM];
		buf[0] = timestamp;
		buf[1] = nbPlaces;
		buf[2] = cat;
		buf[3] = nbEtudiant;

		// socket local
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("socket");
			exit(EXIT_FAILURE);
		}

		// adresse IP serveur
		if ((host = gethostbyname(argv[1])) == NULL) {
			perror("gethostbyname");
			exit(EXIT_FAILURE);
		}

		// préparation adresse serveur
		port = (unsigned short) atoi(argv[2]);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		bcopy(host->h_addr, &addr.sin_addr, host->h_length);

		// demande de connexion serveur
		if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
			perror("connect");
			exit(EXIT_FAILURE);
		}

		// connexion OK
		printf("Connexion acceptée.\n");

		// envoi de la commande
		ssize_t wr;
		if ((wr = write(sock, buf, BUFSIZE)) == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}

		// réception de la réponse de CONCERT
		ssize_t rd;
		if ((rd = read(sock, buf, BUFSIZE)) != BUFSIZE) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		// traitement réponse
		char confirmation = 'z';
		if (buf[1] == nbPlaces) {
			// commande OK, toutes les places demandées sont réservées
			confirmation = 'o';
			printf("Toutes les places sont disponibles.\n");
			// affichage prix
			printf("Prix total : %d€\n", buf[3]);
		} else if ((buf[1] < nbPlaces) && (buf[1] > 0)) {
			// seule une partie des places est disponible
			while ((confirmation != 'o') && (confirmation != 'n')) {
				// affichage prix
				printf("Il ne reste que %d places.\n", buf[1]);
				printf("Prix total : %d€\n", buf[3]);
				printf("Voulez-vous les acheter ? [o/n] ");

				if (fgets(input, sizeof(input), stdin) == NULL) continue;
				confirmation = input[0];
			}
		} else if (buf[1] == 0) {
			// aucune place disponible, fin d'exécution
			confirmation = 'n';
			printf("Il ne reste aucune place.\n");
		} else if (buf[1] > nbPlaces) {
			// c'est une erreur, fin d'exécution
			perror("ACHAT");
			close(sock);
			exit(EXIT_FAILURE);
		}

		if (confirmation == 'o') {
			// demande numéro CB
			printf("Entrez votre numéro de CB : ");
			int numCB;
			while (fgets(input, sizeof(input), stdin)) {
				numCB = strtol(input, &p, 10);

				if (p == input || * p != '\n') {
					printf("Veuillez entrer un numéro de CB valide : ");
				} else break;
			}
			buf[3] = numCB;
		} else if (confirmation == 'n') {
			// refus commande
			// buf[1] contient le nombre de places à restituer
			// on le renvoie en négatif
			buf[1] *= -1;
		}

		// envoi dernière réponse à CONCERT
		if ((wr = write(sock, buf, BUFSIZE)) == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}

		// fermeture connexion à CONCERT
		close(sock);

		// fin de la boucle de demande
		exit(EXIT_SUCCESS);
	}

	return EXIT_SUCCESS;
}
