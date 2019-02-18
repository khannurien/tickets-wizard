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
		// entrée utilisateur
		int nbPlaces;
		int cat;
		int nbEtudiant;

		// invite
		printf("Nouvelle commande. Combien de places ?\n");
		scanf("%d", &nbPlaces);
		printf("Quelle catégorie ?\n");
		scanf("%d", &cat);
		printf("Combien de places en tarif étudiant ?\n");
		scanf("%d", &nbEtudiant);

		// connexion à CONCERT
		printf("Connexion au serveur CONCERT...\n");
		// port client
		unsigned int port;
		// socket client
		int sock;
		struct sockaddr_in addr;
		struct hostent * host;

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

		// TODO: envoi de la commande
		// ...

		// fermeture connexion à PLACES
		close(sock);

		// fin de la boucle de demande
		exit(EXIT_SUCCESS);
	}

	return EXIT_SUCCESS;
}
