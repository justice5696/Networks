
#include "WhoHeader.h"

#define MAXIMUM_DATABASE_ENTRY_LENGTH 50
#define FIRSTFIELD_LENGTH 8
#define SECONDFIELD_LENGTH 20
#define MAX_NUM_RECORDS 100

static FILE *filePointer;
static char *userList[MAX_NUM_RECORDS];
static char **nullPtr = NULL;

void open_database ( char *filename) {
  if ( (filePointer = fopen(filename,"r")) == NULL ) {
    fprintf(stderr, "Error: Can't open file %s\n", filename);
    exit(1);
  }
}

char **lookup_user_names (char *host_name, int *no_of_entries) {

  char *firstField = (char *) malloc(FIRSTFIELD_LENGTH + 1);
  char *secondField = (char *) malloc(SECONDFIELD_LENGTH + 1);
  char *currentLine = (char *) malloc(MAXIMUM_DATABASE_ENTRY_LENGTH + 1);

  int foundAMatch = 0;
  int validHostName = 0;

  *no_of_entries = 0;

  while (fgets(currentLine, MAXIMUM_DATABASE_ENTRY_LENGTH,
	       filePointer) != NULL ) {

    firstField = strtok(currentLine, ":");
    secondField = strtok(NULL, ":");
    if ( (strcmp(firstField, "hostname") != 0) ||
	 (strcmp(secondField, host_name) != 0 ) ) {
      continue;
    }
    
    validHostName = 1;

    while ( fgets(currentLine, MAXIMUM_DATABASE_ENTRY_LENGTH, 
		  filePointer) !=  NULL ) {

      firstField = strtok(currentLine, ":");
      if (( secondField = strtok(NULL, ":")) == NULL) {
	break;
      }

      // If we've reached here, the hostname is valid, and there has been a match.
      if (!foundAMatch) {
	foundAMatch = 1;
      }

      userList[*no_of_entries] = (char *) malloc(100);
      sprintf(userList[(*no_of_entries)++],"%s:%s", firstField, secondField);

    }
    break;
  }

  if (!validHostName) {
    return nullPtr;
  } else if (!foundAMatch) {
    // Make userList nonNull ptr
    userList[0] = "1";
  }
  return userList;
}

void close_database (void) {
  fclose(filePointer);
}




