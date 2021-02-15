#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
// Included to get the support library
#include <calcLib.h>

#define DEBUG
#include "protocol.h"

int main(int argc, char *argv[]){
  
  if (argc!=2) {
    printf("Usage; %s <ip>:<port> \n", argv[0]);
    exit(1);
  }


  char delim[]=":";
  char *Desthost=strtok(argv[1],delim);
  char *Destport=strtok(NULL,delim);

  if (Desthost == NULL || Destport == NULL){
    printf("Missing host or port.\n");
    exit(1);
  };


  int port=atoi(Destport);
#ifdef DEBUG 
  printf("Host %s, and port %d.\n",Desthost,port);
#endif

  struct addrinfo hints, *servinfo, *p;
  int recivedValue;
  int client_socket;
  int numbytes;
  char buf[256];
  struct timeval timeout; 
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;

   memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_UNSPEC; // AF_INET , AF_INET6
   hints.ai_socktype = SOCK_DGRAM; // <<--- TRANSPORT PROTOCOL!!

   
  if ((recivedValue = getaddrinfo(Desthost, Destport, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(recivedValue));
    return 1;
  }

  int i = 0;
  for(p = servinfo; p != NULL; p = p->ai_next) 
  {
    if ((client_socket = socket(p->ai_family, p->ai_socktype,
	  p->ai_protocol)) == -1) 
    {
      printf("Socket creation failed.\n");
      continue;
    }
  
    i++;
    if (p == NULL) 
    {
      fprintf(stderr, "Client failed to create proper socket.\n");
      freeaddrinfo(servinfo);
      exit(1);
    }

    printf("Socket created.\n");
    break;
  }
  
  if(setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0)
  {
    fprintf(stderr, "SO_RCVSOCO failed\n");
  }



  ssize_t sentbytes;
  //init första sendto structen
  struct calcMessage firstStruct;
  firstStruct.type = htons(22);
  firstStruct.message = htonl(0);
  firstStruct.protocol = htons(17);
  firstStruct.major_version = htons(1);
  firstStruct.minor_version = htons(0);

  sentbytes=sendto(client_socket,&firstStruct,sizeof(firstStruct), NULL,  p->ai_addr,p->ai_addrlen);
  printf("Skickade %ld bytes.\n", sentbytes);


  calcProtocol *test = new calcProtocol;
  calcProtocol convertedStruct;  
  calcMessage *recivedMessage = new calcMessage;
  #ifdef DEBUG
  printf("size of test = %ld\n", sizeof(*test));
  #endif

 if ((numbytes = recvfrom(client_socket, test, sizeof(*test), 0,
			p->ai_addr,&p->ai_addrlen)) == -1) 
    {
			if(client_socket!=NULL)
      {
				perror("recvfrom");
			}
			exit(1);
		}
    #ifdef DEBUG
    printf("numbytes = %d\n", numbytes);
    #endif

    if(numbytes == sizeof(calcProtocol))
    {
      
      //ändra så värnden blir läsbara.
      convertedStruct.type = ntohs(test->type);
      convertedStruct.major_version = ntohs(test->major_version);
      convertedStruct.minor_version = ntohs(test->minor_version);
      convertedStruct.id = ntohl(test->id);
      convertedStruct.arith = ntohl(test->arith);
      convertedStruct.inValue1 = ntohl(test->inValue1);
      convertedStruct.inValue2 = ntohl(test->inValue2);
      convertedStruct.inResult = ntohl(test->inResult);
      convertedStruct.flValue1 = test->flValue1;
      convertedStruct.flValue2 = test->flValue2;
      convertedStruct.flResult = test->flResult;


      //printf("recived CalcValue struct:\nType: %d\nmajor_version: %d\nminor_version: %d\nidf: %d\narieth: %d\ninValue1: %d\ninValue2: %d\ninResult: %d\nflValue1: %8.8g\nflValue2: %8.8g\nflResult: %8.8g\n",test->type,test->major_version,test->minor_version,test->id, test->arith, test->inValue1, test->inValue2, test->inResult, test->flValue1,test->flValue2, test->flResult);
      
      printf("recived CalcValue struct:\nType: %d\nmajor_version: %d\nminor_version: %d\nidf: %d\narieth: %d\ninValue1: %d\ninValue2: %d\ninResult: %d\nflValue1: %8.8g\nflValue2: %8.8g\nflResult: %8.8g\n",convertedStruct.type, convertedStruct.major_version, convertedStruct.minor_version, convertedStruct.id, convertedStruct.arith, convertedStruct.inValue1, convertedStruct.inValue2, convertedStruct.inResult, convertedStruct.flValue1, convertedStruct.flValue2, convertedStruct.flResult);


      if(convertedStruct.arith==1)
    {
      convertedStruct.inResult = convertedStruct.inValue1 + convertedStruct.inValue2;
      #ifdef DEBUG
      printf("inAdd svar = %d\n",convertedStruct.inResult);
      #endif
    } 
    else if (convertedStruct.arith==2)
    {
      convertedStruct.inResult = convertedStruct.inValue1 - convertedStruct.inValue2;
      #ifdef DEBUG
      printf("inSubb svar = %d\n",convertedStruct.inResult);
      #endif
    } 
    else if (convertedStruct.arith==3)
    {
      convertedStruct.inResult = convertedStruct.inValue1 * convertedStruct.inValue2;
      #ifdef DEBUG
      printf("inMul svar = %d\n",convertedStruct.inResult);
      #endif
    } 
    else if (convertedStruct.arith==4)
    {
      convertedStruct.inResult = convertedStruct.inValue1 / convertedStruct.inValue2;
      #ifdef DEBUG
      printf("inDiv svar = %d\n",convertedStruct.inResult);
      #endif
    }
    else if(convertedStruct.arith== 5)
    {
      convertedStruct.flResult = convertedStruct.flValue1 + convertedStruct.flValue2;
      #ifdef DEBUG
      printf("flAdd svar = %8.8g\n",convertedStruct.flResult);
      #endif
    }
     else if(convertedStruct.arith== 6)
    {
      convertedStruct.flResult = convertedStruct.flValue1 - convertedStruct.flValue2;
      #ifdef DEBUG
      printf("flSub svar = %8.8g\n",convertedStruct.flResult);
      #endif
    }
     else if(convertedStruct.arith== 7)
    {
      convertedStruct.flResult = convertedStruct.flValue1 * convertedStruct.flValue2;
      #ifdef DEBUG
      printf("flMul svar = %8.8g\n",convertedStruct.flResult);
      #endif
    }
     else if(convertedStruct.arith== 8)
    {
      convertedStruct.flResult = convertedStruct.flValue1 / convertedStruct.flValue2;
      #ifdef DEBUG
      printf("flDiv svar = %8.8g\n",convertedStruct.flResult);
      #endif
    }

    convertedStruct.type = htons(convertedStruct.type);
    convertedStruct.major_version = htons(convertedStruct.major_version);
    convertedStruct.minor_version = htons(convertedStruct.minor_version);
    convertedStruct.id = htonl(convertedStruct.id);
    convertedStruct.arith = htonl(convertedStruct.arith);
    convertedStruct.inValue1 = htonl(convertedStruct.inValue1);
    convertedStruct.inValue2 = htonl(convertedStruct.inValue2);
    convertedStruct.inResult = htonl(convertedStruct.inResult);

    sentbytes=sendto(client_socket,&convertedStruct,sizeof(convertedStruct), NULL,  p->ai_addr,p->ai_addrlen);
    printf("Skickade %ld bytes.\n", sentbytes);

     if ((numbytes = recvfrom(client_socket, recivedMessage, sizeof(*recivedMessage), 0,
			p->ai_addr,&p->ai_addrlen)) == -1) 
    {
			if(client_socket!=NULL)
      {
				perror("recvfrom");
			}
			exit(1);
		}
    #ifdef DEBUG
    printf("numbytes = %d\nMessage(0 = N/A, 1 = OK, 2 = Not OK) = %d\n", numbytes, ntohl(recivedMessage->message));
    #endif


    }
    else
    {
        recivedMessage = (calcMessage *)test;
        if(ntohl(recivedMessage->message) == 2)
        {
          printf("NOT OK");
          exit(2);
        }
        exit(2);
    }



    delete test;
    delete recivedMessage;
}