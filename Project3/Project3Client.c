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
int maxTries;
char * hostname;
int hnsize;
unsigned int fromMsgSize;
int tries = 0;
void CatchAlarm(int ignored);
struct sigaction myAction;
int h;
int caught;
int working;





int main(int argc, char *argv[])
{
    int i;
    char c;
    serverPort = htons(atoi(SERVER_PORT));
    serverHost = SERVER_HOST;
    messageIn = malloc(sizeof(*messageIn));
    fromSize = sizeof(fromAddr);
    h = 1;
    working = 0;
    caught = 0;

    if ((argc < 7) || (argc > 11))
    {
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
                    maxTries = atoi(argv[i+1]);
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

    //Constructing the socket. It is set for UDP communications.
    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        DieWithError((char *)"Socket Construct Failed\n");
    }
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = ResolveName(serverHost);
    serverAddress.sin_port = serverPort;
                                            debug("Socket has been created and memset\n");
    myAction.sa_handler = CatchAlarm;//set signal handler for alarm signal
    if(sigfillset(&myAction.sa_mask)<0)//block everything in handler
    {
      DieWithError((char *)"Error: sigfillset() failed\n");
    }
    myAction.sa_flags = 0;
    if(sigaction(SIGALRM, &myAction, 0) < 0)
    {
      DieWithError((char *)"Error: sigaction() failed\n");
    }

    constructMessage(); //correctly construct the struct based on cmd line  args
                                            debug("Construct message finished\n");
    sendMessage(); //send the struct
    receiveMessage();
    printMessage(messageIn, fromMsgSize);//print the contents of the struct in a formatted way
    close(sock);
    return 0;
}

void constructMessage()
{
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
                                              debug("strncopied hostname into messageOut.data\n");
  messageOut.checkSum = checksum((void *)&messageOut,hnsize+6);
  return;
}

void sendMessage()
{
  if(sendto(sock, (Message *)&messageOut, hnsize + 6, 0, (struct sockaddr *)
    &serverAddress, sizeof(serverAddress)) != (hnsize + 6))
    {
        DieWithError((char *)"send() sent a different number of bytes than expected\n");
    }
                                              debug("message sent\n");
  return;
}

void receiveMessage()
{
alarm(timeout);
  for(;;)
  {
    working = 0;
    if(caught)
    {
      caught = 0;
      tries++;
      debug("Entered alarm(timeout) conditional\n");
      if(tries < maxTries)
      {
        printf("Timed out: %d tries remaining out of %d\n\n", maxTries-tries, maxTries);
        sendMessage();
        alarm(timeout);
      }
      else
      {
        alarm(0);
        DieWithError((char*)"Error: No response");
      }

    debug("Inside receive method\n");
    }
    while((fromMsgSize = recvfrom(sock, messageIn, sizeof(*messageIn), 0, ((struct sockaddr *)&fromAddr), &fromSize)) == -1)
    {
      debug("WHAT THE FUCK\n");
      if(errno == EINTR)
      {
        if(tries < maxTries)
        {
          debug("if errno=EINTR");
          printf("Timed out: %d tries remaining out of %d\n\n", maxTries-tries, maxTries);
          sendMessage();
                                    debug("message sent\n");
          alarm(timeout);
          continue;
        }
        else
        {
          alarm(0);
          DieWithError((char*)"Error: No response");
        }
      }
      else
      {
        DieWithError((char*)"Error: recvfrom() failed\n");
      }
    }
    working = 1;
    debug("received from server\n");
    printf("Handling Client %s\n\n", inet_ntoa(fromAddr.sin_addr));
    unsigned short helpme = checksum((void *)messageIn, fromMsgSize);
    if(DEBUG)
    { if(messageIn->version != messageOut.version){debug("Error: received packet version doesn't match outgoing packet version\n");}
    if(messageIn->type != 4){debug("Error: received packet type doesn't match outgoing packet type \n");}
    if(helpme != 0){debug("Error: Checskum of received packet is not equal to 0.\n");}
    if(messageIn->qID != messageOut.qID){debug("Error: received packet ID doesn't match outgoing packet ID \n");}
    if(serverAddress.sin_addr.s_addr != fromAddr.sin_addr.s_addr){debug("Error: Received a packet from an unknown source.\n" );}}

    if((messageIn->version != messageOut.version)||(messageIn->type != 4)||(helpme != 0)||(messageIn->qID != messageOut.qID)||(serverAddress.sin_addr.s_addr != fromAddr.sin_addr.s_addr))
    {
      debug("entered incorrect message conditional, preparing to memset messageIn\n");
      //caught = 1;
      continue;
    }

    debug("message received and all things correct, retruning from receiveMessage() now\n");
    alarm(0); //correct message received
    return;
  }

}

void CatchAlarm(int ignored)
{
  debug("is this ever entered\n");
  if (!working)
  {
    tries++;
  }
  else
  {
    caught = 1;
  }
}
