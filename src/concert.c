#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
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
 * CONCERT
 * Reçoit les demandes d'ACHAT et les confirme auprès de PLACES.
 * Calcule le prix d'achat et valide les paiements.
 * 
 */

int main(int argc, char * argv[]) {
	if (argc != 5) {
		printf("Usage: %s <port> <PLACES hostname> <PLACES port> <chat port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// initialisation des prix
	prixPlaces[0] = 50;
	prixPlaces[1] = 30;
	prixPlaces[2] = 20;

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
	if (bind(sock, (struct sockaddr *) &srv_addr, srv_addr_lg) == -1) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	// ouverture du service
	if (listen(sock, 10) == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	// ensemble des descripteurs à surveiller
	fd_set from_master, read_fds;
	FD_ZERO(&from_master);
	FD_ZERO(&read_fds);
	FD_SET(0, &from_master); // stdin
	FD_SET(sock, &from_master); // socket ACHAT

	// boucle d'attente de connexion
	while (1) {
		printf("En attente de demande d'ACHAT.\n\n");
		printf("Vous pouvez modifier les prix des places pour les futures commandes en tapant Entrée.\n");
		printf("Attention : pendant cette opération, les nouveaux clients seront mis en attente.\n\n");

		// surveiller l'entrée standard en plus des connexions à ACHAT
		read_fds = from_master;

		int select_fds;
		if ((select_fds = select(sock + 1, &read_fds, NULL, NULL, NULL)) == -1) {
			perror("select");
			exit(EXIT_FAILURE);
		}

		// activité sur stdin ?
		if (FD_ISSET(0, &read_fds)) {
			// on consomme le premier retour chariot
			char c;
			while ((c = getchar()) != '\n' && (c != EOF));

			// vérification de l'entrée utilisateur
			char * p, input[256];
			int prixPlace;

			int i;
			for (i = 0; i < 3; i++) {
				printf("Nouveau prix pour la catégorie %d : ", i + 1);
				while (fgets(input, sizeof(input), stdin)) {
					prixPlace = strtol(input, &p, 10);

					if (p == input || * p != '\n') {
						printf("Veuillez entrer une valeur entière : ");
					} else if (prixPlace <= 0) {
						printf("Veuillez entrer une valeur supérieure à 0 : ");
					} else {
						prixPlaces[i] = prixPlace;
						break;
					}
				}
			}

			printf("Les prix ont bien été modifiés.\n\n");

			// retour à l'exécution
			continue;
		}

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

		// connexion client
		printf("Demande d'ACHAT reçue...\n");

		// réception demande client
		// fork
		int pid;
		if ((pid = fork()) == -1) {
			perror("fork");
			exit(EXIT_FAILURE);
		}

		// un buffer par processus
		int buf[BUFELEM];

		// valeurs attendues dans le buffer
		int timestamp, nbPlaces, cat, nbEtudiant;

		switch(pid) {
			// fils
			case 0:
				// on quitte le socket de RDV
				close(sock);

				// socket UDP
				int chat;
				// adresse locale
				char nomh[50];
				struct sockaddr_in adresseLocale;
				int lgadresseLocale;
				// émetteur
				struct sockaddr_in adresseEmetteur;
				int lgadresseEmetteur;

				// ouverture socket UDP
				if ((chat = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
					perror("socket");
					exit(EXIT_FAILURE);
				}

				// récupération du nom de la machine locale
				if (gethostname(nomh, 50) == -1) {
					perror("gethostname");
					exit(EXIT_FAILURE);
				}

				// préparation adresse locale : port + toutes les IP
				adresseLocale.sin_family = AF_INET;
				adresseLocale.sin_port = htons(atoi(argv[4]));
				adresseLocale.sin_addr.s_addr = htonl(INADDR_ANY);
				lgadresseLocale = sizeof(adresseLocale);
				lgadresseEmetteur = sizeof(adresseEmetteur);

				// attachement de la socket à l'adresse locale
				if ((bind(chat, (struct sockaddr *) &adresseLocale, lgadresseLocale)) == -1) {
					perror("bind");
					exit(EXIT_FAILURE);
				}

				// surveiller TCP et UDP
				fd_set from_service, read_fds_chat;
				FD_ZERO(&from_service);
				FD_ZERO(&read_fds_chat);
				FD_SET(service, &from_master); // socket service
				FD_SET(chat, &from_master); // socket chat

				int select_fds_chat;
				if ((select_fds_chat = select(chat + 1, &read_fds_chat, NULL, NULL, NULL)) == -1) {
					perror("select");
					exit(EXIT_FAILURE);
				}

				// activité sur le chat ?
				if (FD_ISSET(chat, &read_fds)) {
					// boucle de discussion
					char msg_srv[1024];
					char msg_clt[1024];

					fd_set from_chat, read_fds_whileChatting;
					FD_ZERO(&from_chat);
					FD_ZERO(&read_fds_whileChatting);
					FD_SET(0, &from_master); // stdin
					FD_SET(chat, &from_master); // socket chat

					while (1) {
						read_fds_whileChatting = from_chat;

						int select_fds_chat;
						if ((select_fds_chat = select(chat + 1, &read_fds_chat, NULL, NULL, NULL)) == -1) {
							perror("select");
							exit(EXIT_FAILURE);
						}

						if (FD_ISSET(0, &read_fds_whileChatting)) {
							// écriture message
							printf("@%s: ", "dest");
							fgets(msg_srv, 1024, stdin);
							printf("\n");
							// envoi du message
							size_t sd;
							if ((sd = sendto(chat, msg_srv, strlen(msg_srv) + 1, 0, (struct sockaddr *) &adresseEmetteur, (socklen_t) lgadresseEmetteur)) != strlen(msg_srv + 1)) {
								perror("sendto");
								//exit(EXIT_FAILURE);
							}

							continue;
						}

						if (FD_ISSET(chat, &read_fds_whileChatting)) {
							// lecture message
							size_t rv;
							if ((rv = recvfrom(chat, msg_clt, sizeof(msg_clt), 0, (struct sockaddr *) &adresseEmetteur, (socklen_t *) &lgadresseEmetteur)) == -1) {
								perror("recvfrom");
								//return EXIT_FAILURE;
							}

							printf("%s: %s\n", "src", msg_clt);

							continue;
						}
					}
				}

				// traitement de la déconnexion d'ACHAT
				ssize_t rd;

				// lecture de la demande d'ACHAT
				if ((rd = read(service, buf, BUFSIZE)) == BUFSIZE) {
					// vérification de la validité
					timestamp = buf[0];
					nbPlaces = buf[1];
					cat = buf[2];
					nbEtudiant = buf[3];
				} else if (rd == 0) {
					// déconnexion ACHAT
					printf("Client ACHAT déconnecté.\n");

					// fermeture connexion à ACHAT
					close(service);

					// fin de l'exécution
					exit(EXIT_FAILURE);
				}

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

				// envoi de la commande à PLACES
				buf[1] *= -1;
				ssize_t wr;
				if ((wr = write(places_sock, buf, BUFSIZE)) == -1) {
					perror("write");
					exit(EXIT_FAILURE);
				}

				// réception de la réponse de PLACES
				if ((rd = read(places_sock, buf, BUFSIZE)) == -1) {
					perror("read");
					exit(EXIT_FAILURE);
				}

				// traitement de la réponse de PLACES
				float prixFinal = -1;
				int placesEffectives = -1;

				if (buf[1] < 0) {
					// on calcule le prix final et on prépare la proposition à ACHAT
					buf[1] *= -1;
					placesEffectives = buf[1];
					if (nbEtudiant > placesEffectives) nbEtudiant = placesEffectives;
					prixFinal = howMuch(placesEffectives, cat, nbEtudiant);
				} else if (buf[1] > 0) {
					// toute autre valeur est une erreur
					perror("CONCERT");
					exit(EXIT_FAILURE);
				}

				// retransmission à ACHAT
				// buf[1] == 0 si aucune place n'est disponible
				if ((wr = write(service, buf, BUFSIZE)) != BUFSIZE) {
					perror("write");
					exit(EXIT_FAILURE);
				}

				// envoi du prix
				// prixFinal == -1 si aucune place n'est disponible
				if ((wr = write(service, &prixFinal, BUFSIZE)) != BUFSIZE) {
					perror("write");
					exit(EXIT_FAILURE);
				}

				// attente confirmation d'ACHAT
				if ((rd = read(service, buf, BUFSIZE)) < 0) {
					// déconnexion ACHAT
					printf("Client ACHAT déconnecté.\n");
					printf("Restitution des places...\n");
					// envoi désistement
					buf[1] = -placesEffectives;

					if ((wr = write(places_sock, buf, BUFSIZE)) != BUFSIZE) {
						perror("write");
						exit(EXIT_FAILURE);
					}

					// fermeture connexion à PLACES
					close(places_sock);

					// fermeture connexion à ACHAT
					close(service);

					// fin de l'exécution
					exit(EXIT_FAILURE);
				}

				// traitement confirmation d'ACHAT
				// si refus, retour de places à PLACES
				if (buf[1] == -1) {
					// refus commande
					printf("Restitution des places...\n");
					buf[1] = -placesEffectives;
				}

				// envoi de la confirmation à PLACES
				if ((wr = write(places_sock, buf, BUFSIZE)) != BUFSIZE) {
					perror("write");
					exit(EXIT_FAILURE);
				}

				// fermeture connexion à PLACES
				close(places_sock);

				// fermeture connexion à ACHAT
				close(service);

				// fin de l'exécution
				exit(EXIT_SUCCESS);
			default:
				// le socket de service appartient au fils
				close(service);
				break;
		}
	}

	// fermeture socket serveur
	close(sock);

	// fin d'exécution
	return EXIT_SUCCESS;
}
