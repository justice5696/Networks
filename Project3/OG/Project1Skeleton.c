
#include "NetworkHeader.h"

/* function declarations */

main (int argc, char *argv[]) {

  /* There are certainly slicker ways of doing this,
   * but I figured there's no need to make the code
   * any more confusing than it has to be at this point.
   */

  // Argument parsing variables
  char *firstName;
  char *lastName;
  char *serverHost = SERVER_HOST;
  unsigned short serverPort = atoi(SERVER_PORT);
  char *servPortString;
  char c;
  int i;

  if ((argc < 5) || (argc > 7)) {
    printf("Error: Usage Project0Client [-s <server IP>[:<port>]] -f <firstName> -l <lastName>\n");
    exit(1);
  }

  for (i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      c = argv[i][1];

      /* found an option, so look at next
       * argument to get the value of 
       * the option */
      switch (c) {
      case 'f':
	     firstName = argv[i+1];
	break;
      case 'l':
	lastName = argv[i+1];
	break;
      case 's':
	serverHost = strtok(argv[i+1],":");
	if ((servPortString = strtok(NULL, ":")) != NULL) {
	  serverPort = htons(atoi(servPortString));
	}
	break;
      default:
	break;
      }
    }
  }

  /* Your networking code starts here */


}



