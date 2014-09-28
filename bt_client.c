
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

int main (int argc, char * argv[]){

    bt_args_t bt_args;	// structure to capture command-line arguments
    printf("testing, in MAIN(), check whether structure bt_args has been allocated memory, size: %ld\n", sizeof(bt_args));
    printf("testing, in MAIN(), size of bt_info: %ld\n", sizeof(bt_args.bt_info));
    int i;	// loop iterator

    parse_args(&bt_args, argc, argv);

    if (bt_args.verbose) {	// if verbose mode is requested
	printf("Args:\n");
        printf("verbose: %d\n", bt_args.verbose);
        printf("save_file: %s\n", bt_args.save_file);	// display name of file to save to
        printf("log_file: %s\n", bt_args.log_file);		// display name of file to log information to
        printf("torrent_file: %s\n", bt_args.torrent_file);	// metainfo or torrent file being used by bt client

	// print information of all peers
        for (i = 0; i < MAX_CONNECTIONS; i++) {
			if(bt_args.peers[i] != NULL)
                print_peer(bt_args.peers[i]);
        }
    }

    //parse the torrent file to fill up contents of the bt_args structure
    parseTorrentFile(&bt_args);

    if (bt_args.verbose) {
        // print out the torrent file arguments here
	/* bt_args->torrent_file	// .torrent file to read from
	 * bt_args->save_file	// the filename to save to ('name' in .torrent file)
	 * bt_args->bt_info->piece_length	// size of a piece for the file in bytes (power of 2); ('piece length' in .torrent file)
	 * bt_args->bt_info->length	// length of file to be downloaded ('length' in .torrent file)
	 * bt_args->bt_info->num_pieces	// number of pieces file is divided into ()
	 * bt_args->bt_info->piece_hashes	// array of char arrays (20 bytes each) representing each SHA1 hashed piece of file 
						// ('pieces' in torrent file) */
	printf("Torrent file that is being read from: '%s'\n", bt_args.torrent_file);
	printf("Preferred file to save torrent to: '%s'\n", bt_args.save_file);
	printf("Length of file to be downloaded: %d\n", bt_args.bt_info->length);
	printf("Length of each piece of the file in bytes : %d\n", bt_args.bt_info->piece_length);
	printf("Number of pieces the file is divided into: %d\n", bt_args.bt_info->num_pieces);
	printf("Each file piece's hash:\n");
	for (i = 0; i < bt_args.bt_info->num_pieces; i++)
	    printf("\tpiece_hashes[%d]: '%s'\n", i, bt_args.bt_info->piece_hashes[i]);
    }

    // main client loop
    //printf("Starting Main Loop\n");
    /*
	while(1){

        //try to accept incoming connection from new peer
             
        
        // poll current peers for incoming traffic
        //     write pieces to files
        //     update peers' choke or unchoke status
        //     responses to have/havenots/interested etc.
        
        // for peers that are not choked
        //     request pieces from outcoming traffic

        // check livelness of peers and replace dead (or useless) peers
        // with new potentially useful peers
        
        // update peers, 

    }
	*/
	
    return 0;
}
