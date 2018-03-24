/*
NOTE: struct/array/union types have the alignment of their most restrictive member. The first member of each
continuous group of bit fields is typically word-aligned, with all following being continuously packed into following words (though ANSI C requires the latter only for bit-field groups of less than word size).

*/
#include "NetworkHeader.h"


char* serverHost;
char*  database;
unsigned short serverPort;
unsigned int fromSize;
SA serverAddress;
SA fromAddr;
int sock;
Message * messageIn;
Message messageOut;
unsigned int fromMsgSize;
int* numEntries;
int sizel;
int numberOfEntries;
void databaseSearch();


int main(int argc, char *argv[])
{
    int i;
    char c;
    serverPort = htons(atoi(SERVER_PORT));
    messageIn = malloc(sizeof(*messageIn));

    if (argc != 5) {
        printf("Error: Usage Project3Server -p <port> -d <database-file-name>\n");
        exit(1);
    }

    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            c = argv[i][1];

            switch (c)
            {
                case 'd':
                    database = argv[i+1];
                    break;
                case 'p':
                    serverPort = htons(atoi(argv[i+1]));
                    break;
                default:
                    break;
            }
        }
    }


    /* Constructing the socket. It is set for UDP communications.
    */
    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        DieWithError((char *)"Socket Construct Failed\n");
    }
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = serverPort;

    debug("Socket has been created and memset\n");

    if(bind(sock,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
      DieWithError((char *)"Socket Connect Failed\n");
    }

    debug("Socket has been bound to this local address\n");
    /* printf("just after serverAddress costructionn\n");
       fflush(stdout);*/

     //send the struc

    receiveMessage();//receive a message over the socket (as a struct), check that the received address is the expected one
    printMessage();//print the contents of the struct in a formatted way
    close(sock);

    return 0;
}

void constructMessage()
{
  debug("constructing message\n");
  //do i need to hton all of these things??
  messageOut.version = 6; //0110
  messageOut.type = 4; //000
  //messageOut.X = 0; this should be set when doing the database search
  //messageOut.mLength = 1; this should be set when doingg the database search
  messageOut.qID = messageIn->qID; //shoudlnt need to htons this
  messageOut.checkSum = 0;
  //messageOut.data = hostName; this should be set when doing the database search.
  printf("MessageOut.data = %s\n", messageOut.data); // only prints 1 because there are several null characters throughout
  sizel = numberOfEntries*12;
  printf("numberOfEntries: %d\n", numberOfEntries );
  messageOut.checkSum = checksum((void *)&messageOut, sizel+6);
  debug("Finished message.Out checksum\n");
  return;
}

void sendMessage()
{
  if(sendto(sock, (Message *)&messageOut, sizel+6, 0, (struct sockaddr *)
           &fromAddr, sizeof(fromAddr)) != sizel+6)
           {
               DieWithError((char *)"send() sent a different number of bytes than expected\n");
           }

    return;
}

void receiveMessage()
{
   for(;;)
   {
     debug("Inside receive\n");

      fromSize = sizeof(fromAddr);
      //messageIn = malloc(sizeof(Message));
      //if((fromMsgSize = recvfrom(sock, messageIn, sizeof(*messageIn), 0, (struct sockaddr *)&fromAddr, &fromSize)) != sizeof(*messageIn))
      if((fromMsgSize = recvfrom(sock, messageIn, sizeof(*messageIn), 0, (struct sockaddr *)&fromAddr, &fromSize)) < 0 )
      {
         DieWithError((char *) "recvfrom() failed shit\n");
      }

      printf("Version Field: %d(6)\n", messageIn->version);
      printf("Type Field: %d(0)\n", messageIn->type);
      printf("Version Field: %d(0)\n", messageIn->X);
      printf("Length Field: %d(1)\n", messageIn->mgLength);
      printf("qID Field: %d(normal), %d(ntohs), %d(htons)\n", messageIn->qID, ntohs(messageIn->qID), htons(messageIn->qID));
      printf("Checksum: %d(normal), %d(ntohs), %d(htons)\n", messageIn->checkSum, ntohs(messageIn->checkSum), htons(messageIn->checkSum));
      printf("MessageIn.data: %s\n", messageIn->data);


      printf("Handling Client %s\n", inet_ntoa(fromAddr.sin_addr));
      //messageIn->checkSum = ntohs(messageIn->checkSum); //this makes the checkum calculation correct
      //messageIn->qID = ntohs(messageIn->qID);//also makes the checksum calculation corrrect


      unsigned short helpme = checksum((void *)messageIn, fromMsgSize);
      if( helpme != 0)
      {
         printf("Checksum after recv: %d\n", checksum((void *)messageIn, fromMsgSize));
         DieWithError("Checksum of received packets is not 0\n");
      }

      databaseSearch();
      constructMessage(); //correctly construct the struct based on cmd line  args
      sendMessage();
    }
    return;
}


void printMessage(){return;}


void databaseSearch()
{
  debug("Inside the databaseSearch function\n");
  open_database(database);
  debug("Opened Database\n");
  int stupid = 12; //arbitrary
  numEntries = &stupid;
  char ** tempData = lookup_user_names((char *)messageIn->data, numEntries);
  messageOut.X = (tempData==NULL) ? 0 : 1;
  printf("NumEntries: %d\n", *numEntries);
  messageOut.mgLength = *((unsigned char *)numEntries); //might fuck thngs up
  listToSingleArray(tempData, *numEntries, *(&messageOut.data));
  numberOfEntries = *numEntries;
  debug("looked up hostname\n");
  // messageOut.X = 1; need to fix these
  //messageOut.mLength = 1;  need to fix these
  close_database();
}
