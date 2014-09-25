
// standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>			// for getopt(), optarg, optind
#include <string.h>

#include "bt_setup.h"
#include "bt_lib.h"

/**
 * usage(FILE *file) -> void
 *
 * print the usage of this program to the file stream file
 *
 **/

void usage(FILE *file) {
    if(file == NULL) {
        file = stdout;
    }

    fprintf(file,
                    "bt-client [OPTIONS] file.torrent\n"
                    "    -h                        \t Print this help screen\n"
                    "    -b ip                 \t Bind to this ip for incoming connections, ports\n"
                    "                                \t are selected automatically\n"
                    "    -s save_file    \t Save the torrent in directory save_dir (dflt: .)\n"
                    "    -l log_file     \t Save logs to log_file (dflt: bt-client.log)\n"
                    "    -p ip:port        \t Instead of contacing the tracker for a peer list,\n"
                    "                                \t use this peer instead, ip:port (ip or hostname)\n"
                    "                                \t (include multiple -p for more than 1 peer)\n"
                    "    -I id                 \t Set the node identifier to id (dflt: random)\n"
                    "    -v                        \t verbose, print additional verbose info\n");
}

/**
 * __parse_peer(peer_t * peer, char peer_st) -> void
 *
 * parse a peer string, peer_st and store the parsed result in peer
 *
 * ERRORS: Will exit on various errors
 **/
void __parse_peer(peer_t *peer, char *peer_st) {
    char *parse_str;	// string in the form of (IPaddr:port) written as command-line argument after -p
    char *word;		// token grabber variable used with string tokenizer: strtok()
    unsigned short port;	// connection port of peer
    char *ip;	// IP address or hostname of peer
    char id[20];
    char sep[] = ":";	// delimiter separating IP address and port of peer
    int i;	// loop iterator variable

    //need to copy because strtok mangels things
    parse_str = malloc(strlen(peer_st) + 1);
    strncpy( parse_str, peer_st, (strlen(peer_st) + 1) );

    // only can have 2 tokens max, but may have less
    for(word = strtok(parse_str, sep), i = 0; 
			(word && i < 3); 
            word = strtok(NULL, sep), i++) {

		printf("%d:%s\n", i, word);
        switch(i) {
			case 0:	// ip or hostname of peer
				ip = word;
				break;
			case 1: // port of peer
				port = atoi(word);
			default:
				break;
        }
    }

    if (i < 2) {
	fprintf(stderr,"ERROR: Parsing Peer: Not enough values in '%s'\n", peer_st);
        usage(stderr);
        exit(1);
    }

    if(word) {
        fprintf(stderr, "ERROR: Parsing Peer: Too many values in '%s'\n", peer_st);
        usage(stderr);
        exit(1);
    }

    // calculate the id (SHA1 digest), where a 20-byte hex value is placed in 'id' denoting peer id
    calc_id(ip, port, id);

    // build the peer object
    init_peer(peer, id, ip, port);
    
    // free extra memory
    free(parse_str);

    return;
}

/**
 * parse_args(bt_args_t *bt_args, int argc, char *argv[]) -> void
 *
 * parse the command line arguments to bt_client using getopt and
 * store the result in bt_args.
 *
 * ERRORS: Will exit on various errors
 *
 **/
void parse_args(bt_args_t *bt_args, int argc, char *argv[]) {
    int ch;	// ch for each flag
    int n_peers = 0;
    int i;	// loop iterator variable

    /* set the default args */
    bt_args->verbose = 0; // no verbosity
    
    // null save_file, log_file and torrent_file
    memset(bt_args->save_file, 0x00, FILE_NAME_MAX);
    memset(bt_args->torrent_file, 0x00, FILE_NAME_MAX);
    memset(bt_args->log_file, 0x00, FILE_NAME_MAX);
    
    // null out file pointers
    bt_args->f_save = NULL;

    // null bt_info pointer, should be set once torrent file is read
    bt_args->bt_info = NULL;

    //default log file
    strncpy(bt_args->log_file, "bt-client.log", FILE_NAME_MAX);
    
    for(i = 0; i < MAX_CONNECTIONS; i++) {
        bt_args->peers[i] = NULL; // initially NULL
    }

    bt_args->id = 0;	// set bt_client's id to 0
    
    while ((ch = getopt(argc, argv, "hp:s:l:vI:")) != -1) {
        switch (ch) {
		case 'h':	//help
			usage(stdout);
			exit(0);
			break;
		case 'v':	//verbose
			bt_args->verbose = 1;
			break;
		case 's':	//save file
			strncpy(bt_args->save_file, optarg, FILE_NAME_MAX);
			break;
		case 'l':	//log file
			strncpy(bt_args->log_file, optarg, FILE_NAME_MAX);
			break;
		case 'p':	// peer
			n_peers++;
			//check if we are going to overflow
			if (n_peers > MAX_CONNECTIONS) {
			fprintf(stderr,"ERROR: Can only support %d initial peers", MAX_CONNECTIONS);
				usage(stderr);
				exit(1);
			}

			bt_args->peers[n_peers] = malloc( sizeof(peer_t) );
			
			// parse peers
			__parse_peer(bt_args->peers[n_peers], optarg);
			break;
		case 'I':
			bt_args->id = atoi(optarg);
			break;
		default:
		    fprintf(stderr, "ERROR: Unknown option '-%c'\n", ch);
		    usage(stdout);
		    exit(1);
        }
    }

    argc -= optind;
    argv += optind;

    if(argc == 0){
        fprintf(stderr,"ERROR: Remember we need a torrent file? Please try again.\n");
        usage(stderr);
        exit(1);
    }

    // copy torrent file over
    strncpy(bt_args->torrent_file, argv[0], FILE_NAME_MAX);

    return;
}

/**
 * parseTorrentFile(bt_args_t *bt_args) -> void
 * 
 * parse *.torrent file to populate values related to the 'info' part of of the torrent file
 * 
 */
void parseTorrentFile(bt_args_t *bt_args) {
	/*
	bt_args->torrent_file	// .torrent file to read from
	bt_args->save_file	// the filename to save to
	bt_args->bt_info.piece_length	// size of a piece for the file in bytes (power of 2)
	bt_args->bt_info.length	// length of file to be downloaded
	bt_args->bt_info.num_pieces	// number of pieces file is divided into
	bt_args->bt_info.piece_hashes	// array of char arrays (20 bytes each) representing each SHA1 hashed piece of file
	*/
	
	FILE *fp = fopen(bt_args->torrent_file, "r");	// open .torrent file in read only mode
	if (fp == NULL) {
		fprintf(stderr, "ERROR: Could not read file: '%s'\n", bt_args->torrent_file);
		exit(1);
	}
	
	// figure out size of file in bytes
	/*
	fseek(fp, 0, SEEK_END);	// set file pointer to end of file
	long torrentFileSize = ftell(fp);	// store file size
	rewind(fp);	// set file pointer back to beginning of file
	
	// read the entire file and store it in a string
	char *fileContents;		// string to store contents of file in
	fileContents = malloc( torrentFileSize * sizeof(char) + 1 );	// allocate memory equivalent to number of bytes in file
	fread(fileContents, sizeof(char), torrentFileSize, fp);
	fileContents[torrentFileSize] = '\0';	// null terminating the string
	
	char delim[] = ":";	// set delimiters required
	*/
	
	// read torrent file's contents char-by-char
	char ch;	// character holder
	while ( (ch = fgetc(fp)) != EOF ) {
	    
	}
	
	
}