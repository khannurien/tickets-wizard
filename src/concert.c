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
 * CONCERT
 * "Man in the middle"
 * Reçoit les demandes d'ACHAT et les confirme auprès de PLACES.
 * Calcule le prix d'achat et valide les paiements.
 * 
 */

int main(int argc, char * argv[]) {
	if (argc != 4) {
		printf("Usage: %s <port> <PLACES hostname> <PLACES port>", argv[0]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
