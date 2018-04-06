#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include "WhoHeader.h"
#include <errno.h>
#include <signal.h>

//#define SERVER_HOST "141.166.207.143"  //Mathcs02
//#define SERVER_HOST "141.166.207.146"   //Mathcs05
//#define SERVER_HOST "141.166.207.144"    //Mathcs01
#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT "30950"

#define SA struct sockaddr_in

/* Miscellaneous constants */
#define	MAXLINE		4096	/* max text line length */
#define	MAXSOCKADDR  128	/* max socket address structure size */
#define	BUFFSIZE	2048	/* buffer size for reads and writes */
#define	LISTENQ		1024	/* 2nd argument to listen() */
#define SHORT_BUFFSIZE  256     /* For messages I know are short */
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define DEBUG 0

void constructMessage();
void sendMessage();
void receiveMessage();

typedef struct
{
  unsigned short version: 4;
  unsigned short type: 3;
  unsigned short X: 1;
	unsigned char mgLength;
	unsigned short qID: 16; //htons
	unsigned short checkSum: 16;
	char data[BUFFSIZE];
} Message;


unsigned long ResolveName(char name[])
{
  struct hostent *host;
  if((host = gethostbyname(name)) == NULL)
  {
    printf("gethostbyname failed\n");
    fflush(stdout);
  }
  return *((unsigned long *) host->h_addr_list[0]);
}


void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}


//nbytes = total number of bites in the message = sizeof(message)
//unsigned short *ptr = (unsigned short *) message
unsigned short checksum(unsigned short *ptr,int nbytes)
{
    if(DEBUG){printf("checksum entered: nbytes = %d\n", nbytes);}
    unsigned long sum;
    unsigned short oddbyte;
    unsigned short answer;
    sum=0;
    while(nbytes>1)
    {
        sum += *(ptr++);
        nbytes -= 2;
    }
    if(nbytes==1)
    {
        oddbyte=0;
        *((unsigned char*)&oddbyte)=*(unsigned char*)ptr;
        sum+=oddbyte;
    }
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(unsigned short)~sum;
    return(answer);
}

void debug(char * temp)
{
  if(DEBUG)
  {
    printf("%s", temp);
    //fflush(stdout);
  }
}


void listToSingleArray(char** list, int numberOfEntries, char* buff)
{
  int i;
  int j;
  char* tempName;
  long tempTime;
  for(i=0; i < numberOfEntries; i++)
  {
    tempName = strtok(list[i], ":");
    for(j = 0; j <8; j++)
    {
      buff[j+(i*12)] = tempName[j];
      if(j >= strlen(tempName))
      {
        buff[j+(i*12)] = '\0';
      }
    }
    tempTime = atoi(strtok(NULL, ":"));
    memcpy((void *)&buff[8 + (i*12)], (void *)&tempTime, 4);
  }
}

void printMessage(Message *m, int mSize)
{
  if(DEBUG)
  {
    printf("      Packet:%d\n", m->qID);
    printf("Size     Field: %d\n", mSize);
    printf("Version  Field: %d\n", m->version);
    printf("Type     Field: %d\n", m->type);
    printf("X        Field: %d\n", m->X);
    printf("Length   Field: %d\n", m->mgLength);
    printf("qID      Field: %d\n", m->qID);
    printf("Checksum Field: %d\n", m->checkSum);
    printf("Data: \n");
  }

  if(m->X == 0)
  {
    debug("Data Portion Empty");
    printf("Inputted hostname not found in Database.\n");
  }
  else if(m->mgLength == 0)
  {
    printf("No users logged in to inputted hostname.\n");
  }
  else
  {
    printf("%-8s %-5s %-4s","Username", "", "Time\n");
    int i;
    int j;
    for(i = 0; i < ((unsigned short)m->mgLength); i++)
    {
      for(j = 0; j < 8; j++)
      {
        printf("%c", m->data[j + (i * 12)]);
      }

      uint32_t faith;
      printf("%-5s ","");
      memcpy((void *)&faith, (void *)&m->data[8 + (i*12)], 4);
      printf("%d\n", faith);
    };
  }
  return;

}
