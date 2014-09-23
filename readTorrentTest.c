
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    
    FILE *fp = fopen("moby_dick.txt.torrent", "r");
    if (fp == NULL) {
	perror("Could not open file to read");
	exit(1);
    }
    
    // know file size
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);	// set file pointer to beginning of file
        
    // instantiate a buffer of file size length
    char readBuffer[fileSize];
    memset(readBuffer, 0, fileSize);	// zero-out buffer
    fread(readBuffer, sizeof(char), fileSize, fp);
    printf("testing, readBuffer: '%s'\n", readBuffer);
    
    // tokenize the string that was read
    /*
    char *word;
    char delim[] = ":";
    int num;
    for ( word = strtok(readBuffer, delim); word; word = strtok(NULL, delim) ) {
	printf("word: %s\n", word);
	num = atoi(word);
	printf("num: %d\n", num);
    }
    */
    
    int i = 0;
    int marker;
    char *word;
    char delim[] = ":";
    int count = 1;
    if (readBuffer[0] == 'd') {	// all good, first character is 'd' signifying a dictionary
	marker = 1;	// set a marker pointing to 1 char past the first 'd' indicator in torrent file
	for ( word = strtok(readBuffer + marker); word; word = strtok(NULL, delim) ) {
	    
	    if (count % 2 != 0)	// for all odd times of this loop's running, the token correspond to 'keys' and not 'values'
		printf( "key: %d\n", atoi(word) );
	    else
		printf("value: %s\n", word);
	    
	    marker = marker + strlen(word);	// + 1 to go past the delimiter ":"
	    printf("marker: %d\n", marker);
	}
    }
    
    return 0;
}