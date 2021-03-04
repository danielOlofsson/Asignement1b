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
#include <arpa/inet.h>
// Included to get the support library
#include <calcLib.h>

#include "protocol.h"
#define UX

using namespace std;
/* Needs to be global, to be rechable by callback and main */


int nrOfClients = 0;
int maxNrOfClients = 100;
int currentClient = 0;

struct clientAddrArr
{
  struct sockaddr_storage *clientInfo;
  socklen_t ai_addrlen;
  struct calcProtocol *clientCalcProtocol;
  struct timeval time;
};

clientAddrArr savedClients[1000];


bool removeAClient(int index)
{
  bool sorted = false;
  
  for(int i = index; i < nrOfClients-1; i++)
  {
    savedClients[i] = savedClients[i+1];
  }
  nrOfClients--;
  return sorted;
}

/* Call back function, will be called when the SIGALRM is raised when the timer expires. */
void checkJobbList(int signum){
  //As anybody can call the handler, its good coding to check the signal number that called it.§

  printf("Checking for inactive jobs.\nNr Of clients before checking: %d\n",nrOfClients);
  

  if(signum == SIGALRM)
  {
     
    struct timeval comparetime;
    gettimeofday(&comparetime,NULL);

    for(int i = 0; i <nrOfClients; i++)
    {
      if((comparetime.tv_sec - savedClients[i].time.tv_sec) >= 10)
      {
        removeAClient(i);
        printf("inactive clients removed\n");
      }
    }
  }
  
  printf("nrOfclients after checking: %d",nrOfClients);
  #ifdef DEBUG
  
  #endif
  return;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return ((struct sockaddr_in*)sa)->sin_port;
    }

    return ((struct sockaddr_in6*)sa)->sin6_port;
}


void convertCalcMsgToPrintable(struct calcMessage* clientMsg)
{
  clientMsg->type = ntohs(clientMsg->type);
  clientMsg->message = ntohl(clientMsg->message);
  clientMsg->protocol = ntohs(clientMsg->protocol);
  clientMsg->major_version = ntohs(clientMsg->major_version);
  clientMsg->minor_version = ntohs(clientMsg->minor_version);
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
  

void initiateRandomCalcProtocol(calcProtocol *protocolMsg, int idNr)
{
  double fv1,fv2;
  int iv1 ,iv2;
  char msg[256];
  char *op = randomType();

  protocolMsg->type = 1;
  protocolMsg->major_version = 1;
  protocolMsg->minor_version = 0;
  protocolMsg->id = idNr;

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
    protocolMsg->flResult = 0.f;
    protocolMsg->inValue1 = 0;
    protocolMsg->inValue2 = 0;
    protocolMsg->inResult = 0;

    if(strcmp(op,"fadd")==0)
    {
      
      protocolMsg->arith = 5;
    } 
    else if (strcmp(op, "fsub")==0)
    {
      
      protocolMsg->arith = 6;
    } 
    else if (strcmp(op, "fmul")==0)
    {
      
      protocolMsg->arith = 7;
    } 
    else if (strcmp(op, "fdiv")==0)
    {
      
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
      protocolMsg->arith = 1;
    } 
    else if (strcmp(op, "sub")==0)
    {

      protocolMsg->arith = 2;
    }
    else if (strcmp(op, "mul")==0)
    {
      
      protocolMsg->arith = 3;
    } 
    else if (strcmp(op, "div")==0)
    {
      protocolMsg->arith = 4;
    }
    
  }
  convertCalcProtocolToSendable(protocolMsg);
}


bool compareResult(calcProtocol *protocolMsg)
{
  bool correctResult = false;
  double fresult;
  int iresult;
  double quotient;

  if(protocolMsg->arith == 5)
  {
    fresult=protocolMsg->flValue1+ protocolMsg->flValue2;
    
    quotient = abs(fresult - protocolMsg->flResult);
    if(quotient < 0.0001)
    {
      correctResult = true;
    }
    #ifdef DEBUG
    printf("fresult = %8.8g\nDelta = %8.8g\n",fresult, quotient);
    #endif
  }
  else if (protocolMsg->arith == 6)
  {
    fresult=protocolMsg->flValue1 - protocolMsg->flValue2;
    quotient = abs(fresult - protocolMsg->flResult);
    if(quotient < 0.0001)
    {
      correctResult = true;
    }
    #ifdef DEBUG
    printf("fresult = %8.8g\nDelta = %8.8g\n",fresult, quotient);
    #endif
  } 
  else if (protocolMsg->arith == 7)
  {
    fresult=protocolMsg->flValue1 * protocolMsg->flValue2;
    quotient = abs(fresult - protocolMsg->flResult);
    if(quotient < 0.0001)
    {
      correctResult = true;
    }
    #ifdef DEBUG
    printf("fresult = %8.8g\nDelta = %8.8g\n",fresult, quotient);
    #endif
  } 
  else if (protocolMsg->arith == 8)
  {
    fresult=protocolMsg->flValue1 / protocolMsg->flValue2;
    quotient = abs(fresult - protocolMsg->flResult);
    if(quotient < 0.0001)
    {
      correctResult = true;
    }
    #ifdef DEBUG
    printf("fresult = %8.8g\nDelta = %8.8g\n",fresult, quotient);
    #endif
  }
  else if(protocolMsg->arith == 1)
  {
    iresult=protocolMsg->inValue1 + protocolMsg->inValue2;
    if(iresult == protocolMsg->inResult)
    {
      correctResult = true;
    }
    #ifdef DEBUG
    printf("Inresult = %d\n",iresult);
    #endif
  } 
  else if (protocolMsg->arith == 2)
  {
    iresult=protocolMsg->inValue1 - protocolMsg->inValue2;
    if(iresult == protocolMsg->inResult)
    {
      correctResult = true;
    }
    #ifdef DEBUG
    printf("Inresult = %d\n",iresult);
    #endif
  }
  else if (protocolMsg->arith == 3)
  {
    iresult=protocolMsg->inValue1 * protocolMsg->inValue2;
    if(iresult == protocolMsg->inResult)
    {
      correctResult = true;
    }
    #ifdef DEBUG
    printf("Inresult = %d\n",iresult);
    #endif
  } 
  else if (protocolMsg->arith == 4)
  {
    iresult=protocolMsg->inValue1 / protocolMsg->inValue2;
    if(iresult == protocolMsg->inResult)
    {
      correctResult = true;
    }
    #ifdef DEBUG
    printf("Inresult = %d\n",iresult);
    #endif
  }
  return correctResult;
}


int main(int argc, char *argv[]){
  
  /* Do more magic */

  if (argc!=2) 
  {
    printf("Usage; %s <ip>:<port> \n", argv[0]);
    exit(1);
  }
  initCalcLib();
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
  
  printf("done.\n");

  socklen_t addrlen;
  struct addrinfo hints, *servinfo, *p;
  int recivedValue;
  int serverSocket;
  int numbytes;
  int yes = 1;
  int idCounter = 0;

  char readableIp1[INET6_ADDRSTRLEN];
  char readableIp2[INET6_ADDRSTRLEN];
  int port1, port2;


  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; // AF_INET , AF_INET6
  hints.ai_socktype = SOCK_DGRAM; // <<--- TRANSPORT PROTOCOL!!
  hints.ai_flags = AI_PASSIVE; 

  calcMessage okMsg, notOkMsg;
  okMsg.major_version = 1;
  okMsg.minor_version = 0;
  okMsg.message = 1;
  okMsg.protocol = 17;
  okMsg.type = 2;

  convertCalcMsgToSendable(&okMsg);

  notOkMsg.major_version = 1;
  notOkMsg.minor_version = 0;
  notOkMsg.message = 2;
  notOkMsg.protocol = 17;
  notOkMsg.type = 2;

  convertCalcMsgToSendable(&notOkMsg);

  //UX Hade 2 st notSupported.type = 2 fixat till protocol = 17 samt ändrade msg till 0 från 2

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
        sizeof(int)) == -1) 
    {
      perror("setsockopt");
      exit(1);
    }

    if (bind(serverSocket, p->ai_addr, p->ai_addrlen) == -1) 
    {
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

  calcProtocol *calcPtr = new calcProtocol;
  calcProtocol *tempCalcPtr = new calcProtocol;
  calcMessage *msgPtr = new calcMessage;

  
  
  bool clientFound = false;
  sockaddr_storage clientIn;
  socklen_t clientinSize = sizeof(clientIn);
  while(1)
  {
    
    memset(calcPtr,0,sizeof(*calcPtr));
    numbytes = recvfrom(serverSocket, calcPtr, sizeof(*calcPtr), 0,
    (struct sockaddr *)&clientIn,&clientinSize);

    if(numbytes < 0) 
    {
      perror("revfrom error");
      continue;
		}
    
    //Client Skickar sitt svar.... 
    if(numbytes == sizeof(calcProtocol))
    {
      
      convertCalcProtocolToPrintable(calcPtr);
      
      #ifdef DEBUG
      printf("Arith= %d\n",calcPtr->arith);
      #endif

      inet_ntop(clientIn.ss_family, get_in_addr((struct sockaddr *)&clientIn),
			readableIp2, sizeof(readableIp2));
      //Fixa for loopen
      for(int i = 0; i < nrOfClients; i++)
      {
        inet_ntop(savedClients[i].clientInfo->ss_family, get_in_addr((struct sockaddr *)savedClients[i].clientInfo),
			  readableIp1, sizeof(readableIp1));
      
        
        port1 = get_in_port((struct sockaddr *)&clientIn);
        port2 = get_in_port((struct sockaddr *)savedClients[i].clientInfo);

        #ifdef UX
        convertCalcProtocolToPrintable(savedClients[i].clientCalcProtocol);
        #endif
        if(savedClients[i].clientCalcProtocol->id == calcPtr->id && strcmp(readableIp1,readableIp2) == 0 && clientFound != true
        && port1 == port2)
        {
          clientFound = true;
          currentClient = i;
        }
      }
      
      
      if(clientFound == true)
      {
        //should calculate the result and see if the client got the same answer.
        clientFound = false;
        if(compareResult(calcPtr) == true)
        {
          //send a calcmsg that says it was ok
          
          numbytes = sendto(serverSocket,&okMsg,sizeof(calcMessage), 0, (struct sockaddr*)&clientIn,clientinSize);
          if(numbytes < 0)
          {
            fprintf(stderr,"sendTo error");
            break;
          }
          for(int i = 0; i < nrOfClients; i++)
          {
            //hade removeAClient(i) ändrade till remove a Client till currentClient UX
            removeAClient(currentClient);
            
          }
          continue;
        }
        else
        {
          //send A calcMsg that says it was rejected
          #ifdef DEBUG
          printf("before sending Notok msg\n");
          #endif
          numbytes = sendto(serverSocket,&notOkMsg,sizeof(calcMessage), 0, (struct sockaddr*)&clientIn,clientinSize);
          if(numbytes < 0)
          {
            fprintf(stderr,"sendTo error");
            break;
          }
          continue;
        }
      }
      else
      {
        printf("Clients need to send a calcMsg first, client not found\n");
        numbytes = sendto(serverSocket,&notOkMsg,sizeof(calcMessage), 0, (struct sockaddr*)&clientIn,clientinSize);
        if(numbytes < 0)
        {
          fprintf(stderr,"sendTo error");
          break;
        }
        continue;
      }
    }
    else if(numbytes == sizeof(calcMessage))//either supports or not
    {
      msgPtr = (calcMessage *)calcPtr;
      convertCalcMsgToPrintable(msgPtr);
      
      if(checkIfSupports(msgPtr) == true)
      {
        //protocol supported initiated and send calcprotocol

        savedClients[nrOfClients].clientInfo = &clientIn;
        savedClients[nrOfClients].ai_addrlen = sizeof(clientinSize);
        
    
        //intiates and converts to sendable
        initiateRandomCalcProtocol(tempCalcPtr, idCounter++);
        #ifdef DEBUG
        printf("before sending Supports  msg\n");
        #endif
        numbytes = sendto(serverSocket,tempCalcPtr,sizeof(*tempCalcPtr), 0, (struct sockaddr*)&clientIn,clientinSize);
        if(numbytes < 0)
        {
          fprintf(stderr,"sendTo error");
          break;
        }
        else
        {
          savedClients[nrOfClients].clientCalcProtocol = tempCalcPtr;
          gettimeofday(&savedClients[nrOfClients++].time,NULL);
        }
        continue;
      }
      else
      {
        //protocol not supoorted send back a calcMessage
        printf("before sending does not support msg\n");
        numbytes = sendto(serverSocket,&notOkMsg,sizeof(calcMessage), 0, (struct sockaddr*)&clientIn,clientinSize);
        if(numbytes < 0)
        {
          fprintf(stderr,"sendTo error");
          break;
        }
        #ifdef DEBUG
        printf("Sent Not supported msg size: %d", numbytes);
        #endif
        continue;
      }
    }
    else // UX tillagd koll på om storleken == calcmsg och tillagd else fall där skräp kommer.
    {
      printf("Rubbish msg need to be calcMSG first!\n");
      numbytes = sendto(serverSocket,&notOkMsg,sizeof(calcMessage), 0, (struct sockaddr*)&clientIn,clientinSize);
      if(numbytes < 0)
      {
        fprintf(stderr,"sendTo error");
        break;
      }
      continue;
    }
  }
  delete calcPtr;
  delete msgPtr;
  return(0);
}