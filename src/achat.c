#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
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
 *  - s'il reste des places en nombre insuffisant, propose à l'acheteur de prendre les places restantes ;
 *  - s'il ne reste plus de place disponible, informe l'acheteur et termine la transation.
 * 
 */

int main(int argc, char * argv[]) {
	if (argc != 4) {
		printf("Usage: %s <CONCERT hostname> <CONCERT port> <CONCERT chat port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// boucle de demande
	while (1) {
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

		// identifiant de demande
		int timestamp = (int) time(NULL);
		// entrée utilisateur
		int nbPlaces;
		int cat;
		int nbEtudiant;

		// vérification de l'entrée utilisateur
		char * p, input[256];
		int doWeChat = 0;
		
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
		
		// test chat
		doWeChat = 1;

		if (doWeChat) {
			// socket émetteur
			int chat;
			struct sockaddr_in adresseReceveur;
			int lgadresseReceveur;
			struct hostent * hote;

			// création socket
			if ((chat = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
				perror("socket");
				exit(EXIT_FAILURE);
			}

			// recherche adresse IP distante
			if ((hote = gethostbyname(argv[1])) == NULL) {
				perror("gethostbyname");
				exit(EXIT_FAILURE);
			}

			// préparation adresse distante
			adresseReceveur.sin_family = AF_INET;
			adresseReceveur.sin_port = htons(atoi(argv[3]));
			bcopy(hote->h_addr, &adresseReceveur.sin_addr, hote->h_length);
			lgadresseReceveur = sizeof(adresseReceveur);

			// buffer
			char msg_clt[1024];
			char msg_srv[1024];

			fd_set from_chat, read_fds_whileChatting;
			FD_ZERO(&from_chat);
			FD_ZERO(&read_fds_whileChatting);
			FD_SET(0, &from_chat); // stdin
			FD_SET(chat, &from_chat); // socket chat

			// boucle de discussion
			while (1) {
				read_fds_whileChatting = from_chat;

				int select_fds_chat;
				if ((select_fds_chat = select(chat + 1, &read_fds_whileChatting, NULL, NULL, NULL)) == -1) {
					perror("select");
					exit(EXIT_FAILURE);
				}

				if (FD_ISSET(0, &read_fds_whileChatting)) {
					// écriture message
					printf("@%s: ", "dest");
					fgets(msg_srv, 1024, stdin);

					// quitter ?
					if (strcmp(msg_srv, "/quit") == 0) break;

					printf("\n");
					// envoi du message
					size_t sd;
					if ((sd = sendto(chat, msg_clt, strlen(msg_clt) + 1, 0, (struct sockaddr *) &adresseReceveur, (socklen_t) lgadresseReceveur)) != strlen(msg_clt + 1)) {
						perror("sendto");
						//exit(EXIT_FAILURE);
					}

					continue;
				}

				if (FD_ISSET(chat, &read_fds_whileChatting)) {
					// lecture message
					size_t rv;
					if ((rv = recvfrom(chat, msg_srv, sizeof(msg_srv), 0, (struct sockaddr *) &adresseReceveur, (socklen_t *) &lgadresseReceveur)) == -1) {
						perror("recvfrom");
						//return EXIT_FAILURE;
					}

					printf("%s: %s\n", "src", msg_clt);

					continue;
				}
			}

			// fermeture chat
			close(chat);

			// nouvelle commande
			continue;
		}

		// buffer
		int buf[BUFELEM];
		buf[0] = timestamp;
		buf[1] = nbPlaces;
		buf[2] = cat;
		buf[3] = nbEtudiant;

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

		// réception du prix
		float prixFinal;
		if ((rd = read(sock, &prixFinal, sizeof(float))) != sizeof(float)) {
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
			printf("Prix total : %f€\n", prixFinal);
		} else if ((buf[1] < nbPlaces) && (buf[1] > 0)) {
			// seule une partie des places est disponible
			while ((confirmation != 'o') && (confirmation != 'n')) {
				// affichage prix
				printf("Il ne reste que %d places.\n", buf[1]);
				printf("Prix total : %f€\n", prixFinal);
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
			buf[1] = -1;
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
