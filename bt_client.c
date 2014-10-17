
// standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// libraries for networking stuff
#include <netinet/in.h>				// for struct sockaddr_in
#include <netinet/ip.h>				// ip header library (must come before ip_icmp.h)
#include <netinet/ip_icmp.h> 		// icmp header
#include <arpa/inet.h>				// internet address library
#include <netdb.h>					// for hostent struct
#include <sys/socket.h>				// for socket operations
#include <sys/types.h>
#include <signal.h>

#include "bt_lib.h"
#include "bt_setup.h"

static int seeder_count;    // track number of seeders; scope: private to bt_client.c

int main (int argc, char * argv[]) {

    bt_args_t bt_args; // structure to capture command-line arguments
    int i;	// loop iterator

    parse_args(&bt_args, argc, argv);

    if (bt_args.verbose) {	// if verbose mode is requested
        printf("Args:\n");
        printf("\tverbose: %d\n", bt_args.verbose);
        printf("\tsave_file: %s\n", bt_args.save_file);	// display name of file to save to
        printf("\tlog_file: %s\n", bt_args.log_file);		// display name of file to log information to
        printf("\ttorrent_file: %s\n", bt_args.torrent_file);	// metainfo or torrent file being used by bt client

        // print information of all peers
        for (i = 0; i < MAX_CONNECTIONS; i++) {
            if(bt_args.peers[i] != NULL)
                print_peer(bt_args.peers[i]);
            else
                break;
        }
    }

    // initialize & allocate memory to bt_info struct that's inside bt_args
    bt_args.bt_info = (bt_info_t *) malloc(sizeof(bt_info_t));

    // parse the torrent file to fill up contents of the bt_info structure with required information from the 'info' dictionary in .torrent file
    parse_torrent_file(&bt_args, bt_args.bt_info);
    /*printf("testing, bt_args.bt_info->name: '%s'\n", bt_args.bt_info->name);    // FAILS
    printf("testing, bt_args.bt_info->length: %d\n", bt_args.bt_info->length);  */

/*    // construct handshake information for each peer in swarm
    for (i = 0; i < MAX_CONNECTIONS; i++) {
        if (bt_args.peers[i] != NULL) {
            fill_handshake_info(bt_args.peers[i], bt_args.bt_info);
        } else
            break;
    }*/

    if (bt_args.bind == 1) {    // client needs to bind to seeder
        printf("testing, bt_args.bind_info: '%s'\n", bt_args.bind_info);

        // separate IPaddr:port from string following '-b'; generate bt client's ID
        tokenize_seeder(&bt_args);

        // get a seeder's socket that'll be listening for incoming leecher connections
        init_seeder(bt_args.peers[seeder_count++]);
        
        printf("LISTENING on peer: %s", bt_args.bind_info);
        printf("; id: ");
        for (i = 0; i < ID_SIZE; i++) {
            printf("%02x", bt_args.id[i]);
        }
        printf("\n");   // line feed

    }
    printf("testing, seeder_count: %d\n", seeder_count);

    // main client loop
    // printf("Starting Main Loop\n");
    /*
	while(1){

        // try to accept incoming connection from new peer
             
        // poll current peers for incoming traffic
        // write pieces to files
        // update peers' choke or unchoke status
        // responses to have/havenots/interested etc.
        
        // for peers that are not choked
        //     request pieces from outcoming traffic

        // check livelness of peers and replace dead (or useless) peers
        // with new potentially useful peers
        
        // update peers, 

    }
	*/
	
    return 0;
}
