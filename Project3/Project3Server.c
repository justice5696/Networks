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
    if (argc != 5)
    {
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
    //Constructing the socket. It is set for UDP communications.
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
    receiveMessage();//receive a message over the socket (as a struct), check that the received address is the expected one
    close(sock);

    return 0;
}

void constructMessage()
{
                                      debug("constructing message\n");
  messageOut.version = 6; //0110
  messageOut.type = 4; //4
  messageOut.qID = messageIn->qID;
  messageOut.checkSum = 0;
  sizel = numberOfEntries*12;
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
      if((fromMsgSize = recvfrom(sock, messageIn, sizeof(*messageIn), 0, (struct sockaddr *)&fromAddr, &fromSize)) < 0 )
      {
         DieWithError((char *) "recvfrom() failed\n");
      }
                                       if(DEBUG){printMessage(messageIn, fromMsgSize);}
      printf("Handling Client %s\n\n", inet_ntoa(fromAddr.sin_addr));
      unsigned short helpme = checksum((void *)messageIn, fromMsgSize);
      if( helpme != 0)
      {
        debug("Error: Checskum of received packet is not equal to 0.\n");
        receiveMessage();
      }
      else
      {
        databaseSearch();
        constructMessage();
        sendMessage();
      }
    }
    return;
}




void databaseSearch()
{
                                    debug("Inside the databaseSearch function\n");
  open_database(database);
                                    debug("Opened Database\n");
  int stupid = 0; //arbitrary
  numEntries = &stupid;
  char ** tempData = lookup_user_names((char *)messageIn->data, numEntries);
  numberOfEntries = *numEntries;
  messageOut.X = (tempData==NULL) ? 0 : 1;
                                  debug((char *)&numberOfEntries);
                                  debug(":NumEntries\n");
  messageOut.mgLength = *((unsigned char *)&numberOfEntries);
  listToSingleArray(tempData, numberOfEntries, *(&messageOut.data));
                                  debug("looked up hostname\n");
  close_database();
}
