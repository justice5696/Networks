#include "NetworkHeader.h"


char* serverHost;
unsigned short serverPort;
unsigned int fromSize;
SA serverAddress;
SA fromAddr;
int sock;
const char* hostName;
Message * messageIn;
Message messageOut;
int timeout;
int maxRetries;
char * hostname;
int hnsize;
unsigned int fromMsgSize;





int main(int argc, char *argv[])
{
    int i;
    char c;
    serverPort = htons(atoi(SERVER_PORT));
    serverHost = SERVER_HOST;
    messageIn = malloc(sizeof(*messageIn));

    if ((argc < 7) || (argc > 11)) {
        printf("Error: Usage Project3Client [-h <server IP>] [-p <port>] -t <timeout> -i <max-retries> -d <hostName>\n");
        exit(1);
    }

    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            c = argv[i][1];

            switch (c)
            {
                case 't':
                    timeout = atoi(argv[i+1]);
                    break;
                case 'i':
                    maxRetries = atoi(argv[i+1]);
                    break;
                case 'd':
                    hostname = argv[i+1];
                    break;
                case 'h':
                    serverHost = argv[i+1];
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
    serverAddress.sin_addr.s_addr = ResolveName(serverHost);
    serverAddress.sin_port = serverPort;

    /* printf("just after serverAddress costructionn\n");
       fflush(stdout);*/
    debug("Socket has been created and memset\n");


    constructMessage(); //correctly construct the struct based on cmd line  args
    debug("Construct message finished\n");

    sendMessage(); //send the struct
    printMessage(messageIn, fromMsgSize);//print the contents of the struct in a formatted way
    debug("made it past recive\n");
    close(sock);

    return 0;
}

void constructMessage()
{
  //do i need to hton all of these things???
  memset(&messageOut, 0, sizeof(messageOut));

                                              debug("messageOut memset to 0 \n");

  messageOut.version = 6; //0110
  messageOut.type = 0; //000
  messageOut.X = 0; //0
  messageOut.mgLength = 1;
                                              debug("VERTYX and length set\n");
  time_t t;
  srand((unsigned) time(&t));
  messageOut.qID = rand() % 65536;
                                         debug("random number generated and set \n");
  messageOut.checkSum = 0;

  hnsize = strlen(hostname);
  hnsize = hnsize%2 == 0 ? hnsize +2 : hnsize +1;
  strcpy(messageOut.data, hostname);
  messageOut.data[hnsize-1] = '\0';
  //strncpy(messageOut.data, hostName ,sizeof( *hostName));
                                              debug("strncopied hostname into messageOut.data\n");
  messageOut.checkSum = checksum((void *)&messageOut,hnsize+6);
                                              printf("Size struct: %ld\n", sizeof(messageOut));
  printf("qID Field: %d(normal), %d(ntohs), %d(htons)\n", messageOut.qID, ntohs(messageOut.qID), htons(messageOut.qID));
  printf("Checksum: %d(normal), %d(ntohs), %d(htons)\n", messageOut.checkSum, ntohs(messageOut.checkSum), htons(messageOut.checkSum));

  return;
}

void sendMessage()
{
  //cast message

  if(sendto(sock, (Message *)&messageOut, hnsize + 6, 0, (struct sockaddr *)
           &serverAddress, sizeof(serverAddress)) != (hnsize + 6))
           {
               DieWithError((char *)"send() sent a different number of bytes than expected\n");
           }
  debug("message sent\n");
  receiveMessage();//receive a message over the socket (as a struct), check that the received address is the expected one

  return;
}

void receiveMessage()
{
    //if retransmission timer doesnt fail -> then return, else -> sendMessage()
    debug("Inside receive method\n");
    fromSize = sizeof(fromAddr);
    //if((responseLength = recvfrom(sock, (Message *)&messageIn, sizeof(messageIn), 0, (struct sockaddr *)&fromAddr, &fromSize))!= sizeof(messageIn))
    if((fromMsgSize = recvfrom(sock, messageIn, sizeof(*messageIn), 0, (struct sockaddr *)&fromAddr, &fromSize)) < 0)
    {
      //set retransmission timer so that if the
       DieWithError((char *) "recvfrom() failed\n");
    }
    debug("received from server\n");




    printf("Handling Client %s\n", inet_ntoa(fromAddr.sin_addr));
    //messageIn->checkSum = ntohs(messageIn->checkSum); //this makes the checkum calculation correct
    //messageIn->qID = ntohs(messageIn->qID);//also makes the checksum calculation corrrect


    unsigned short helpme = checksum((void *)messageIn, fromMsgSize);
    if( helpme != 0)
    {
       printf("Checksum after recv: %d\n", checksum((void *)messageIn, fromMsgSize));
       DieWithError("Checksum of received packets is not 0\n");
    }
    if(messageIn->qID != messageOut.qID)
    {
        DieWithError((char *)"Error: received packet ID doesn't match outgoing packet ID \n" );
    }
    if(serverAddress.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
      DieWithError((char *)"Error: Received a packet from an unknown source.\n" );
    }

    ///check everything, if anything is wrong, receive again

    ///if timer runs out, send again
    return;
}


void printMessage(Message *m, int mSize)
{

  printf("      Packet:%d\n", m->qID);
  printf("Size     Field: %d\n", mSize);
  printf("Version  Field: %d\n", m->version);
  printf("Type     Field: %d\n", m->type);
  printf("Version  Field: %d\n", m->X);
  printf("Length   Field: %d\n", m->mgLength);
  printf("qID      Field: %d\n", m->qID);
  printf("Checksum Field: %d\n", m->checkSum);
  printf("Data: \n");


  if(m->X == 0)
  {
    printf("Data Portion Empty");
    return;
  }
  else
  {
    printf("%-8s %-5s %-4s","Username", "", "Time\n");
    //print two columns of user data
    int i;
    int j;
    for(i = 0; i < ((unsigned short)m->mgLength); i++)
    {
      for(j = 0; j < 8; j++)
      {
        printf("%c", m->data[j + (i * 12)]);
      }
      printf("%-5s ","");
      char temp[4];

      temp = m.data[8 + (i*12)];
      printf("%s \n", temp);
    };
  }
  return;

}
