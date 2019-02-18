#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
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
 * CONCERT
 * "Man in the middle"
 * Reçoit les demandes d'ACHAT et les confirme auprès de PLACES.
 * Calcule le prix d'achat et valide les paiements.
 * 
 */

/**
 * Taille du buffer
 */
#define BUFSIZE sizeof(int)

int main(int argc, char * argv[]) {
	if (argc != 4) {
		printf("Usage: %s <port> <PLACES hostname> <PLACES port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// buffer
	int buf;

	// en tant que serveur CONCERT
	// port serveur
	int port;
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
		exit(EXIT_FAILURE);
	}

	// ouverture du service
	if (listen(sock, 10) == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	// boucle d'attente de connexion
	while(1) {
		printf("En attente de ACHAT...\n");

		// attente client
		clt_addr_lg = sizeof(clt_addr);
		if ((service = accept(sock, (struct sockaddr *) &clt_addr, (socklen_t *) &clt_addr_lg)) == -1) {
			if (errno == EINTR) {
				// réception d'un signal
				continue;
			} else {
				// erreur
				perror("accept");
				exit(EXIT_FAILURE);
			}
		}

		// réception demande client
		// fork
		int pid;
		if ((pid = fork()) == -1) {
			perror("fork");
			exit(EXIT_FAILURE);
		}

		switch(pid) {
			// fils
			case 0:
				if (read(service, &buf, BUFSIZE) == BUFSIZE) {
					printf("Reçu : %d.\n", buf);
				} else {
					perror("read");
					exit(EXIT_FAILURE);
				}

				// connexion client
				printf("Demande d'ACHAT reçue...\n");

				// connexion à PLACES
				printf("Connexion au serveur PLACES...\n");

				// en tant que client PLACES
				// port client
				unsigned int places_port;
				// socket client
				int places_sock;
				struct sockaddr_in places_addr;
				struct hostent * host;

				// socket local
				if ((places_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
					perror("socket");
					exit(EXIT_FAILURE);
				}

				// adresse IP serveur
				if ((host = gethostbyname(argv[2])) == NULL) {
					perror("gethostbyname");
					exit(EXIT_FAILURE);
				}

				// préparation adresse serveur
				places_port = (unsigned short) atoi(argv[3]);
				places_addr.sin_family = AF_INET;
				places_addr.sin_port = htons(places_port);
				bcopy(host->h_addr, &places_addr.sin_addr, host->h_length);

				// demande de connexion serveur
				if (connect(places_sock, (struct sockaddr *) &places_addr, sizeof(places_addr)) == -1) {
					perror("connect");
					exit(EXIT_FAILURE);
				}

				// connexion OK
				printf("Connexion acceptée.\n");

				// TODO: envoi de la commande
				// ...

				// fermeture connexion à PLACES
				close(places_sock);

				// fin de l'exécution
				exit(EXIT_SUCCESS);
		}
	}

	// fermeture socket serveur
	close(sock);

	// fin d'exécution
	return EXIT_SUCCESS;
}
