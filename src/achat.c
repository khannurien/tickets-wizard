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
		printf("Usage: %s <CONCERT hostname> <CONCERT port>", argv[0]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
