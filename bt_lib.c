#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>	//internet address library
#include <netdb.h>  // for struct hostent
#include <sys/socket.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <arpa/inet.h>

#include <openssl/sha.h>	// for using SHA1() function for hashing

#include "bt_lib.h"
#include "bt_setup.h"

/**
 * calc_id() clubs the IP address and port number of peer into a single string. Then, 
 * fills the peer 'id' with a SHA1 digest computed on the clubbed string
 */
void calc_id(char *ip, unsigned short port, char *id) {
    char data[256];		// data for which SHA1 digest is to be made
    int len;			// length of data
    
    /* format print to store up to 256 bytes (characters) including the terminating '\0' (null) character in 'data'.
     * snprintf() returns the index of the char that it places the '\0' at; basically returns the length of the string copied
     */
    len = snprintf(data, 256, "%s%u", ip, port);    // example data = localhost7000

    /* id is just the SHA1 of the ip and port string
     * SHA1()'s signature:
     * unsigned char *SHA1(const unsigned char *dataString, unsigned long dataLength, unsigned char *SHA1digest);
     */
    SHA1( (unsigned char *) data, len, (unsigned char *) id );

    return;
}

/**
 * init_peer(peer_t *peer, int id, char *ip, unsigned short port) -> int
 *
 * initialize the peer_t structure peer with an id, ip address, and a
 * port. Further, it will set up the sockaddr such that a socket
 * connection can be more easily established.
 *
 * Return: 0 on success, negative values on failure. Will exit on bad
 * ip address.
 *     
 **/
int init_peer(peer_t *peer, char *id, char *ip, unsigned short port) {
        
    struct hostent *hostinfo;	// instantiate hostent struct that contains information like IP address, host name, etc.
	
    // set the host id and port for reference
    memcpy(peer->id, id, ID_SIZE);  // SHA1 hash of peer IP & port is stored as peer struct's 'id'
    peer->port = port;
        
    // get the host by name
    if( (hostinfo = gethostbyname(ip) ) == NULL) {
        perror("gethostbyname failure, no such host?");
        herror("gethostbyname");	// prints error message associated with the current host
        exit(1);
    }
    
    // zero out the peer's sock address structure before filling in its details
    bzero(&(peer->sockaddr), sizeof(peer->sockaddr));
            
    // set the family to AF_INET, i.e., Internet Addressing
    peer->sockaddr.sin_family = AF_INET;
	
    // copy the address from h_addr field of 'hostinfo' to the peer's socket address
    bcopy( (char *) (hostinfo->h_addr), 
                (char *) &(peer->sockaddr.sin_addr.s_addr),
                hostinfo->h_length);
        
    // encode the port to network-byte order to store in sockaddr_in struct
    peer->sockaddr.sin_port = htons(port);
    
    return 0;	// success
}

/**
 * print_peer(peer_t *peer) -> void
 *
 * print out debug info of a peer
 *
 **/
void print_peer(peer_t *peer) {
    int i;	// loop iterator

    if (peer) {
		printf("peer: %s:%u ", 
				inet_ntoa(peer->sockaddr.sin_addr),		// converts a network address to a dots-and-numbers format string
				peer->port);
        printf("id: ");
        
		for (i = 0; i < ID_SIZE; i++) {
            printf("%02x", peer->id[i]);	// convert to 40-byte hex string
        }
		
        printf("\n");
    }
}

void fill_handshake_info(peer_t *peer, bt_info_t *bt_info) {

    // null all handshake_info_t structure contents at first
    memset(peer->hs_info.protocol, 0x00, sizeof(peer->hs_info.protocol));
    memset(peer->hs_info.reserved, 0x00, sizeof(peer->hs_info.reserved));
    memset(peer->hs_info.info_hash, 0x00, sizeof(peer->hs_info.info_hash));
    memset(peer->hs_info.peerID, 0x00, sizeof(peer->hs_info.peerID));

    char temp[20];  // temporary char array
    sprintf(temp, "%c", 19);    // first byte is ASII character 19 (decimal)
    strcat(temp, "BitTorrent Protocol");    // concatenate '19' byte with "BitTorrent Protocol" string
    
    // fill those 20-bytes as protocol information in peer's handshake information
    strcpy( peer->hs_info.protocol, temp );   // peer->sockaddr.sin_addr.s_addr
    printf("testing, protocol: '%s'\n", peer->hs_info.protocol);

    // do not need to 'reserved bytes', so leave them as null for now

    // printf("testing, bt_info->name: '%s'\n", bt_args->bt_info->name);
    // calculate SHA1 of "suggested file name" specified in *.torrent file
    // SHA1( (unsigned char *) bt_info->name, strlen(bt_info->name), (unsigned char *) peer->hs_info.info_hash );

}