#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * Tickets Wizard
 * Vincent Lannurien
 * UBO Brest, 2019
 * GNU GPL v3
 * 
 * PLACES
 * Gestionnaire de places disponibles pour un concert.
 * Traite les demandes de CONCERT de manière séquentielle.
 * 
 * Répond à l'application CONCERT :
 * 	- dans le cas d'une réservation (entier négatif) ;
 * 	- dans le cas d'un désistement (entier positif).
 * 
 * Retourne à CONCERT :
 * 	- le nombre de places demandées effectivement réservées (entier négatif) ;
 * 	- une erreur :
 * 	  - plus de place disponible (zéro) ;
 * 	  - le nombre de places encore disponibles, mais pas réservées (entier négatif).
 * 
 */

#define BUFSIZE sizeof(int)
int NBPLACES = 100;

int main(int argc, char * argv[]) {
	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// buffer
	int buf;

	// port
	unsigned short port;
	// socket de RDV
	int sock, service;
	// adresse de RDV
	struct sockaddr_in srv_addr;
	int srv_addr_lg;
	// adresse client
	struct sockaddr_in clt_addr;
	int clt_addr_lg;

	// création socket RDV
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	// préparation adresse locale
	port = (unsigned short) atoi(argv[1]);
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(port);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	srv_addr_lg = sizeof(srv_addr);

	// attachement socket RDV
	if (bind(sock, (struct sockaddr *) & srv_addr, srv_addr_lg) == -1) {
		perror("bind");
		return EXIT_FAILURE;
	}

	// ouverture du service
	if (listen(sock, 10) == -1) {
		perror("listen");
		return EXIT_FAILURE;
	}

	// boucle d'attente de connexion
	while(1) {
		printf("PLACES\n");
		printf("Disponibles : %d\n", NBPLACES);

		// attente client
		clt_addr_lg = sizeof(clt_addr);
		if ((service = accept(sock, (struct sockaddr *) &clt_addr, (socklen_t *) &clt_addr_lg)) == -1) {
			if (errno == EINTR) {
				// réception d'un signal
				continue;
			} else {
				// erreur
				perror("accept");
				return EXIT_FAILURE;
			}
		}

		// connexion client
		printf("Demande de CONCERT reçue...\n");

		// réception demande client
		ssize_t rd;
		if ((rd = read(service, &buf, BUFSIZE)) == BUFSIZE) {
			printf("Reçu : %d.\n", buf);
		} else {
			perror("read");
			return EXIT_FAILURE;
		}

		// traitement requête
		ssize_t wr;
		if (buf < 0) {
			// commande
			if (NBPLACES == 0) {
				// pas de dispo
				// retourne 0
				buf = 0;
				printf("Pas de place disponible.\n");
			} else if (NBPLACES <= buf) {
				// pas assez de dispo
				// retourne -NBPLACES
				buf = -NBPLACES;
				printf("Pas assez de places disponibles.\n");
			} else {
				// commande OK
				// retourne buf
				printf("Commande OK.\n");
			}
		} else if (buf > 0) {
			// désistement
			// retourne buf
			NBPLACES += buf;
			printf("Retour de %d places dans le tiroir !\n", buf);
		}

		// réponse au client
		if ((wr = write(service, &buf, BUFSIZE)) == -1) {
			perror("write");
			return EXIT_FAILURE;
		}

		printf("Envoyé : %d.\n", buf);

		// fermeture connexion client
		close(service);
	}

	// fermeture socket RDV
	close(sock);

	// fin d'exécution
	return EXIT_SUCCESS;
}
