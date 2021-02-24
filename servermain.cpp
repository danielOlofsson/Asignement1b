#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

/* You will to add includes here */
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
// Included to get the support library
#include <calcLib.h>

#include "protocol.h"


using namespace std;
/* Needs to be global, to be rechable by callback and main */
int loopCount=0;
int terminate=0;


/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobbList(int signum){
  // As anybody can call the handler, its good coding to check the signal number that called it.§

  printf("Let me be, I want to sleep.\n");

  if(loopCount>20)
  {
    printf("I had enough.\n");
    terminate=1;
  }
  
  return;
}

void convertCalcMsgToPrintable(struct calcMessage* clientMsg)
{
  clientMsg->type = ntohs(clientMsg->type);
  clientMsg->message = ntohl(clientMsg->message);
  clientMsg->protocol = ntohs(clientMsg->protocol);
  clientMsg->major_version = ntohs(clientMsg->major_version);
  clientMsg->major_version = ntohs(clientMsg->minor_version);
}

void convertCalcMsgToSendable(struct calcMessage* serverMsg)
{
  serverMsg->type = htons(serverMsg->type);
  serverMsg->message = htonl(serverMsg->message);
  serverMsg->protocol = htons(serverMsg->protocol);
  serverMsg->major_version = htons(serverMsg->major_version);
  serverMsg->major_version = htons(serverMsg->minor_version);
}

void convertCalcProtocolToPrintable(struct calcProtocol* clientMsg)
{
  clientMsg->type = ntohs(clientMsg->type);
  clientMsg->major_version = ntohs(clientMsg->major_version);
  clientMsg->minor_version = ntohs(clientMsg->minor_version);
  clientMsg->id = ntohl(clientMsg->id);
  clientMsg->arith = ntohl(clientMsg->arith);
  clientMsg->inValue1 = ntohl(clientMsg->inValue1);
  clientMsg->inValue2 = ntohl(clientMsg->inValue2);
  clientMsg->inResult = ntohl(clientMsg->inResult);
}

void convertCalcProtocolToSendable(calcProtocol *serverMsg)
{
  serverMsg->type = htons(serverMsg->type);
  serverMsg->major_version = htons(serverMsg->major_version);
  serverMsg->minor_version = htons(serverMsg->minor_version);
  serverMsg->id = htonl(serverMsg->id);
  serverMsg->arith = htonl(serverMsg->arith);
  serverMsg->inValue1 = htonl(serverMsg->inValue1);
  serverMsg->inValue2 = htonl(serverMsg->inValue2);
  serverMsg->inResult = htonl(serverMsg->inResult);
}


bool checkIfSupports(calcMessage* clientMsg)
{
  bool supports = false;
  if(clientMsg->major_version == 1 && clientMsg->message == 0 && clientMsg->minor_version == 0 && clientMsg->protocol == 17 && clientMsg->type == 22)
  {
    supports = true;
  }
  return supports;
}
  

void initiateRandomCalcProtocol(calcProtocol *protocolMsg)
{
  double fv1,fv2,fresult;
  int iv1 ,iv2 ,iresult;
  char msg[256];
  char *op = randomType();

  protocolMsg->type = 1;
  protocolMsg->major_version = 1;
  protocolMsg->minor_version = 0;
  
  memset(msg, 0, sizeof(msg));

  if(op[0] == 'f')
  {
    #ifdef DEBUG
    printf("Float\t");
    #endif
    fv1=randomFloat();
    fv2=randomFloat();


    protocolMsg->flValue1 = fv1;
    protocolMsg->flValue2 = fv2;
    protocolMsg->flResult = (double)0;
    protocolMsg->inValue1 = 0;
    protocolMsg->inValue2 = 0;
    protocolMsg->inResult = 0;

    if(strcmp(op,"fadd")==0)
    {
      //fresult=fv1+fv2;
      protocolMsg->arith = 5;
    } 
    else if (strcmp(op, "fsub")==0)
    {
      //fresult=fv1-fv2;
      protocolMsg->arith = 6;
    } 
    else if (strcmp(op, "fmul")==0)
    {
      //fresult=fv1*fv2;
      protocolMsg->arith = 7;
    } 
    else if (strcmp(op, "fdiv")==0)
    {
      //fresult=fv1/fv2;
      protocolMsg->arith = 8;
    }
    #ifdef DEBUG
    printf("Servers result: %8.8g",fresult);
    #endif
  }
  else
  {
    iv1 = randomInt();
    iv2 = randomInt();
    protocolMsg->inValue1 = iv1;
    protocolMsg->inValue2 = iv2;
    protocolMsg->inResult = 0;
    protocolMsg->flValue1 = (double)0;
    protocolMsg->flValue2 = (double)0;
    protocolMsg->flResult = (double)0;
    if(strcmp(op,"add")==0)
    {
      //iresult=iv1+iv2;
      protocolMsg->arith = 1;
    } 
    else if (strcmp(op, "sub")==0)
    {
      //iresult=iv1-iv2;
      protocolMsg->arith = 2;
    }
    else if (strcmp(op, "mul")==0)
    {
      //iresult=iv1*iv2;
      protocolMsg->arith = 3;
    } 
    else if (strcmp(op, "div")==0)
    {
      //iresult=iv1/iv2;
      protocolMsg->arith = 4;
    }
    #ifdef DEBUG
    printf("Servers result: %d",iresult);
    #endif
  }
  convertCalcProtocolToSendable(protocolMsg);
}


int main(int argc, char *argv[]){
  
  /* Do more magic */

  if (argc!=2) 
  {
    printf("Usage; %s <ip>:<port> \n", argv[0]);
    exit(1);
  }

  char delim[]=":";
  char *Desthost=strtok(argv[1],delim);
  char *Destport=strtok(NULL,delim);

  if (Desthost == NULL || Destport == NULL){
    printf("Missing host or port.\n");
    exit(2);
  };


  /* 
     Prepare to setup a reoccurring event every 10s. If it_interva, or it_value is omitted, it will be a single alarm 10s after it has been set. 
  */
  struct itimerval alarmTime;
  alarmTime.it_interval.tv_sec=10;
  alarmTime.it_interval.tv_usec=10;
  alarmTime.it_value.tv_sec=10;
  alarmTime.it_value.tv_usec=10;

  /* Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of */
  signal(SIGALRM, checkJobbList);
  setitimer(ITIMER_REAL,&alarmTime,NULL); //Start/register the alarm. 

  
  while(terminate==0){
    printf("This is the main loop, %d time.\n",loopCount);
    sleep(1);
    loopCount++;
  }

  printf("done.\n");

  struct sockaddr_storage remoteaddr; // klient address  
  socklen_t addrlen;
  struct addrinfo hints, *servinfo, *p, *clientinfo;
  int recivedValue;
  int serverSocket;
  int numbytes;
  int yes = 1;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; // AF_INET , AF_INET6
  hints.ai_socktype = SOCK_DGRAM; // <<--- TRANSPORT PROTOCOL!!
  hints.ai_flags = AI_PASSIVE; 

  if ((recivedValue = getaddrinfo(Desthost, Destport, &hints, &servinfo)) != 0) 
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(recivedValue));
    exit(1);
  }

  for(p = servinfo; p != NULL; p = p->ai_next) 
  {
    if ((serverSocket = socket(p->ai_family, p->ai_socktype,
	  p->ai_protocol)) == -1) 
    {
      printf("Socket creation failed.\n");
      continue;
    }
  
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes,
        sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(serverSocket, p->ai_addr, p->ai_addrlen) == -1) {
			close(serverSocket);
			perror("listener: bind");
			continue;
		}

    #ifdef DEBUG
    printf("Socket created.\n");
    #endif
    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) 
  {
    fprintf(stderr, "Client failed to create proper socket.\n");
    freeaddrinfo(servinfo);
    exit(1);
  }

  addrlen = sizeof(addrlen);

  int klient_socket;
  int dL, sL;

  struct calcMessage firstStruct;
  firstStruct.type = htons(22);
  firstStruct.message = htonl(0);
  firstStruct.protocol = htons(17);
  firstStruct.major_version = htons(1);
  firstStruct.minor_version = htons(0);

  calcProtocol *calcPtr = new calcProtocol;
  calcMessage *msgPtr = new calcMessage;

  struct clientAddrArr
  {
    struct sockaddr *ai_addr;
    socklen_t ai_addrlen;
  };
  clientAddrArr savedClients[1];

  while(1)
  {
    numbytes = recvfrom(serverSocket, calcPtr, sizeof(*calcPtr), 0,
    p->ai_addr,&p->ai_addrlen);
    
    if(numbytes < 0) 
    {
      perror("revfrom error");
      continue;
		}
    else
    {
      savedClients[0].ai_addr = p->ai_addr;
      savedClients[0].ai_addrlen = p->ai_addrlen;
      break;
    }

    //Client Skickar sitt svar.... 
    if(numbytes == sizeof(calcProtocol))
    {
      
    }
    else
    {
      msgPtr = (calcMessage *)calcPtr;
      convertCalcMsgToPrintable(msgPtr);
      if(checkIfSupports(msgPtr) == true)
      {
        //protocol supported initiate and send calcprotocol
      }
    }


  }

  delete calcPtr;
  delete msgPtr;
  return(0);
}
