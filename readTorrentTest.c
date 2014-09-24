
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fastForward(char *, FILE *);
void storeForward(char *, FILE *);

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
        
    while ( (ch = fgetc(fp)) != EOF) {	// read file up to the end
	
	if (ch == 'd') {	/* if 'd' is found in the file, we need to check each 'key' from then on to look for the key: 'info' coz it 
				 * is the 'info' dictionary that contains all data relevant to us.
				 * every dictionary has an 'e' as ending indicator corresponding to its character "begin" indicator 'd' */
	    
	    // fair assumption made that a 'dictionary' can have no other but only a 'string' in its 'key' places
	    	    
	    // look for 'key': "info"
	    while ( (ch = fgetc(fp)) != 'e') {	// as long as the dictionary does not end
		storeForward(&ch, fp);
	    }
	}
	
	/* else the file could begin with a string (e.g. 4:spam), integer (e.g. i3e) or a list (e.g. l8:pleasant5:smilee i.e. ["pleasant", "smile"]) 
	 * whatever it is, and it will not matter to us coz we should only be alarmed at the sight of another 
	 * 'dictionary' hoping that it is the 'info' dictionary.
	 */
	
	// CASE: if file starts with "integer", just get past the integer coz it is insignificant to us
	if (ch == 'i') {
	// the next 'e' will indicate the end of the integer
	    while ( (ch = fgetc(fp)) != 'e') {
		    continue;	// traverse through integer's digits
	    }
	}
	    
	// CASE: if file starts with "list", just get past that list too (list begins with character 'l')
	if (ch == 'l') {
	    while ( (ch = fgetc(fp)) != 'e') {	// while the list does not end,
		fastForward(&ch, fp);
	    }
	    //printf("2. testing, ch: %c\n", fgetc());
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
    
    switch (*c) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    num = atoi(c);	// store first digit
	    num = constructNum(num);	// construct a natural number
	    while ((*c = fgetc(fptr)) != ':') {
		num = atoi(c);	// more than one digit found
		num = constructNum(num);
	    }
	default:
	    break;
    }
    fseek(fptr, num, SEEK_CUR);	// move file pointer ahead by length of string (num) after ':'
}

void storeForward(char *c, FILE *fptr) {
    
    int num = 0;
    char buf[] = "";	// temporary string-holder
    
    switch(*c) {
	case '4':	// hope for an "info" string
	    num = atoi(c);
	    num = constructNum(num);
	    while ((*c = fgetc(fptr)) != ':') {
		num = atoi(c);	// more than one digit found; no chance of "info" being in there
		num = constructNum(num);	// still need to construct that number to fast-forward without storing anythingd
	    }
	    if (num == 4) {	/* if number is indeed the single-digit '4' and not something like '472', then
				 * check if 'key' is equal to "info" */
		int i = 0;
		while (i < 4) {	// construct the word that follows the ':'
		    *c = fgetc(fptr);
		    strcat(buf, c);	// keep appending a char to previous
		    i++;
		}
		buf[i] = '\0';	// null termination of string
		
		// now compare string with "info"
		char infoString[5];
		strcpy(infoString, "info");
		if ( strcmp(buf, infoString) == 0) {
		    printf("We've finally found 'info'\n");
		}
	    }
	case '0': case '1': case '2': case '3': 
	case '5': case '6': case '7': case '8': case '9':
	default:
	    break;
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