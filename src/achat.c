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

#define BUFSIZE 4

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

		// invite
		// TODO: tester les valeurs entrées...
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

		// buffer
		char buf[BUFSIZE];
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
		if ((wr = write(sock, &buf, BUFSIZE)) == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}

		// réception de la réponse de CONCERT
		ssize_t rd;
		if ((rd = read(sock, &buf, BUFSIZE)) != BUFSIZE) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		// TODO: traitement réponse
		// ...

		// TODO: demande numéro CB
		// ...

		// TODO: réponse validation paiement
		// ...

		// fermeture connexion à CONCERT
		close(sock);

		// fin de la boucle de demande
		exit(EXIT_SUCCESS);
	}

	return EXIT_SUCCESS;
}
