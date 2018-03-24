#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>  // For isspace

void open_database ( char *filename);
char **lookup_user_names (char *host_name, int *no_of_entries);
void close_database (void);
void made(int index);

