
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fastForward(char *, FILE *);
void storeForward(char *, FILE *);
int handleNumbers(char *, FILE *);

static int finalNum;
int constructNum(int);

int main(int argc, char *argv[]) {
    
    FILE *fp = fopen("testTorrParserFile.torrent", "r");
    if (fp == NULL) {
	perror("Could not open file to read");
	exit(1);
    }
    
    /* APPROACH: "COPYING FILE CONTENTS TO STRING"
    // know file size
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);	// set file pointer to beginning of file
        
    // define a buffer of file size length
    char readBuffer[fileSize + 1];	// 1 more than the file size to accomodate the null pointer
    memset(readBuffer, 0, fileSize);	// zero-out buffer
    fread(readBuffer, sizeof(char), fileSize, fp);
    readBuffer[fileSize] = '\0';	// null termination
    printf("testing, readBuffer: '%s'\n", readBuffer);
    
    // string tokenize (strtok) does not seem like it could work
    int i;
    char buf[] = "";	// empty string
    for (i = 0; i < strlen(readBuffer); i++) {	// traverse entire string that was read from file
	if (readBuffer[0] == 'd') {	// first character in string
	    // first character in string may be a 'd' so we'd know that it is a dictionary and
	    // therefore, should also have an ending 'e' as last character at end of string
	    
	    i++;	// simply step over to next character
	}
	
	// else the file could begin with a string, integer or a list whatever suits, and it will not matter to us coz we
	// should only be alarmed at the sight of another 'dictionary' hoping that it is the 'info' dictionary.
	// whatever may be the case here, simply read the length of string (some number) up to occurence of a ":"
	 
	while ( readBuffer[i] != ":") {	// read until next occurence of ":"
	    
	    i++;	    
	}
    }
    */
    
    // APPROACH: "SIMPLY READING CONTENTS OF FILE USING FGETC"
    char ch;
    while ( (ch = fgetc(fp)) != EOF ) {	// read file up to the end
	
	if (ch == 'd') {	/* if 'd' is found in the file, we need to check each 'key' from then on to look for the key: 'info' coz it 
				 * is the 'info' dictionary that contains all data relevant to us.
				 * every dictionary has an 'e' as ending indicator corresponding to its character "begin" indicator 'd' */
	    
	    // a fair assumption made here is that a 'dictionary' can have no other but only a 'string' in its 'key' places
	    
	    // look for 'key': "info"
	    while ( (ch = fgetc(fp)) != 'e')	// as long as the dictionary does not end
		storeForward(&ch, fp);

	}
	
	/* the file could begin with a string (e.g. 4:spam), integer (e.g. i3e) or a list (e.g. l8:pleasant5:smilee i.e. 
	 * ["pleasant", "smile"]) whatever it is, and it will not matter to us coz we should only be alarmed at the sight of another 
	 * 'dictionary' hoping that it is the 'info' dictionary.
	 */
	
	// CASE: if file finds an "integer"not inside a dictionary, just get past the integer coz it is insignificant to us
	if (ch == 'i') {
	// the next 'e' will indicate the end of the integer
	    while ( (ch = fgetc(fp)) != 'e') {
		    continue;	// traverse through integer's digits
	    }
	}
		    
	// CASE: if file finds a "list", just get past that list too (list begins with character 'l')
	if (ch == 'l') {
	    
	    while ( (ch = fgetc(fp)) != 'e') {	// while the list does not end,
		fastForward(&ch, fp);		
	    }
	    
	}
	
	// CASE: if file starts with "string", just get past that string too (string begins with a 'number')
	fastForward(&ch, fp);	/* move on in the file
				 * NOTE: will not fast-forward in file if a "number" is not found; 
				 * 'offset' is set to 0 in fastForward() for such a case
				 */
	
    }
    
    return 0;
}

/**
 * fastForward(char *, FILE *)
 * 	Used to simply get past the bytes following a ':'. Getting past the bytes or characters means that we do not 
 * 	need to store any information from the file and therefore, are just moving ahead in the file.
 * 
 * @param char* "the character in the file that was last read"
 * @param FILE* "pointer to the file being read"
 * 
 * @return void
 */
void fastForward(char *c, FILE *fptr) {
    int num = 0;	/* temporary integer holder; 
			 * num = 0 is also useful when offset needs to be 0 (does not fast-forward) */
    
    finalNum = 0;	// reset static variable
    
    num = handleNumbers(c, fptr);
    
    fseek(fptr, num, SEEK_CUR);	// move file pointer ahead by length of string (num) after ':'
    
}

/**
 * This function checks exactly which digits appear in a bencoded string before a ':' and 
 * accordingly keeps passing each digit to the constructNum() function.
 * @param char* "the character that has the digit"
 * @param FILE* "pointer to the file being read"
 * 
 * @return int "the fully constructed natural number"
 */
int handleNumbers(char *chr, FILE *fpr) {
    int num = 0;
    switch (*chr) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    num = atoi(chr);	// store first digit
	    num = constructNum(num);	// construct a natural number
	    while ((*chr = fgetc(fpr)) != ':') {
		num = atoi(chr);	// more than one digit found
		num = constructNum(num);
	    }
	default:
	    break;
    }
    
    return num;
}

/**
 * storeForward(char *, FILE *)
 * 	This function is just like fastForward() except that instead of moving the file pointer ahead without storing any 
 * file contents, storeForward() stores strings in a temporary buffer.
 * 
 * @param char* "the character last read in file"
 * @param FILE* "pointer to the file being read"
 * 
 * @return void
 */
void storeForward(char *c, FILE *fptr) {
    
    int num = 0;
    finalNum = 0;	// reset static variable
    char *buffer;	// temporary string holder
    switch(*c) {
	case '4':	// if we get a '4', we hope to see an "info" string
	    num = atoi(c);
	    num = constructNum(num);
	    while ((*c = fgetc(fptr)) != ':') {
		num = atoi(c);	// more than one digit found; no chance of "info" being in there
		num = constructNum(num);	// still need to construct that number to fast-forward without storing anythingd
	    }
	    printf("testing, num: %d\n", num);
	    if (num == 4) {	/* if number is indeed the single-digit '4' and not something like '472', then
				 * check if 'key' is equal to "info" */
		int i;
		buffer = (char *) malloc(num * sizeof(char) + 1);	// 1 extra allocation for null pointer
		memset(buffer, 0, sizeof(buffer));	// zero-out buffer before anything is read into it
		for (i = 0; i < num; i++) {	// store string of length 4, into a buffer
		    *c = fgetc(fptr);
		    buffer[i] = *c;
		}
		printf("testing, buffer: '%s'\n", buffer);
		
		if (strcmp(buffer, "info") == 0) {	// 'buffer' contents equal to "info"?
		    free(buffer);	// for now, release memory allocated to 'buffer'; we will reallocate this memory later
		    
		    if ( (*c = fgetc(fptr)) != 'd') {	// but if 'info' is not followed by a dictinary ('d')
			// may be what follows is an integer, a list or a string; handle all such cases
			switch(*c) {
			    case 'l':	// list encountered
				while ( (*c = fgetc(fptr) != 'e') )
				    continue;
				break;
			    case 'i':	// integer encountered
				while ( (*c = fgetc(fptr) != 'e') )
				    continue;
				break;
			    fastForward(c, fptr);	// simply move ahead to find another 'info' sometime later
			    default:
				handleNumbers(c, fptr);	// string encountered
				break;
			}
		    } else {	// case where a dictionary follows after 'info'
			
			while ( (*c = fgetc(fptr)) != 'e' ) {	// read through the whole 'info' dictionary until it ends
			    num = handleNumbers(c, fptr);
			    buffer = (char *) malloc(num * sizeof(char));	// reallocate memory for 'buffer' for new strings to be read in it
			    memset(buffer, 0, sizeof(buffer));	// flush buffer
			    for (i = 0; i < num; i++) {
				*c = fgetc(fptr);
				buffer[i] = *c;
			    }
			    handleInfoContents(buffer, c, fptr);
			}
			
		    }
		}
	    }
	    break;
	case '0': case '1': case '2': case '3': 
	case '5': case '6': case '7': case '8': case '9':
	    num = handleNumbers(c, fptr);
	    break;
	default:
	    break;
    }
    
    printf("1. testing, num: %d\n", num);
    fseek(fptr, num, SEEK_CUR);	// offset file pointer ahead    
}

void handleInfoContents(char *buf, char *chr, FILE *fpr) {

    if ( strcmp(buf, "length") == 0 ) {
	int num = 0;
	finalNum = 0;	// reset static 'finalNum'
	if ( (*chr = fgetc(fpr)) == 'i' ) {
	    while ( (*chr = fgetc(fpr)) != 'e' ) {
		num = atoi(chr);
		num = constructNum(num);
	    }
	    printf("2. testing, num: %d\n", num);
	    
	}
	
    } else if ( strcmp(buffer, "name") == 0 ) {
    } else if ( strcmp(buffer, "piece length") == 0 ) {
    } else if ( strcmp(buffer, "pieces") == 0 ) {
    }
}

/**
 * constructNum(int)
 * 	Constructs a natural number by successively appending each digit to the previous one on successive calls and
 * 	forms a non-single digit natural number. When called only once, will return a single-digit natural number.
 * 
 * @param int "each single digit parsed in file that can be appended to the previous one"
 * 
 * @return int "the entire non-single digit or single digit natural number"
 */
int constructNum(int n) {
    return ( finalNum = (finalNum * 10 + n) );
}