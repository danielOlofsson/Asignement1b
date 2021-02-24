#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

/* You will to add includes here */
#include <poll.h>
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

  if(loopCount>20){
    printf("I had enough.\n");
    terminate=1;
  }
  
  return;
}

int initListenerSocket(const char* destHost, const char* destPort)
{
  int listenSocket;
  int recivedValue;
  int yes=1;
  struct addrinfo hints, *servinfo, *p;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if ((recivedValue = getaddrinfo(destHost, destPort, &hints, &servinfo)) != 0) 
  {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(recivedValue));
      exit(1);
  }
  
  for(p = servinfo; p != NULL; p = p->ai_next)
  {
    listenSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listenSocket < 0) { 
      continue;
    }
    
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listenSocket, p->ai_addr, p->ai_addrlen) < 0) 
    {
      close(listenSocket);
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo); 

    
    if (p == NULL) 
    {
      return -1;
    }

    
    if (listen(listenSocket, 10) == -1) 
    {
      return -1;
    }

  return listenSocket;
}


void addNewConnection(struct pollfd *pfds[], int newSocket, int* nrOfClients, int *maxNrOfClients)
{
  //add space if needed
  if(*nrOfClients == *maxNrOfClients)
  {
    *maxNrOfClients *=2;

    *pfds = realloc(*pfds, sizeof(**pfds) * (*maxNrOfClients));
  }

  (*pfds)[*nrOfClients].events = POLLIN;

  (*nrOfClients)++;

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


  int listener; // lysnar socketen
  int newSocket;
  struct sockaddr_storage remoteaddr; // klient address  
  socklen_t addrlen;

  char buffer[300];
  char remoteIp[INET6_ADDRSTRLEN];

  int nrOfClients = 0;
  int maxNrOfClients = 10;
  size_t pollfdSize = sizeof(pollfd) * maxNrOfClients;
  struct pollfd *pfds = malloc(pollfdSize);

  listener = initListenerSocket(Desthost,Destport);

  if(listener == -1)
  {
    fprintf(stderr,"error initiating listening socket");
    exit(3);
  }

  pfds[0].fd = listener;
  pfds[0].events = POLLIN;

  nrOfClients++;

  for(;;)
  {
    int poll_count = poll(pfds,nrOfClients, -1);

    if(poll_count == -1)
    {
      perror("poll error");
      exit(4);
    }

    for(int i = 0; i < nrOfClients; i++)
    {
      //cheking if any socket is ready to read
      if(pfds[i].revents & POLLIN) 
      {
        if(pfds[i].fd == listener)
        {
          //handels new connection if listener is reday

          addrlen = sizeof(remoteaddr);
          newSocket = accept(listener,(struct sockaddr*)&remoteaddr,&addrlen);

          if(newSocket == -1)
          {
            perror("accept error");
          }
          else
          {
            addNewConnection(&pfds,newSocket,&nrOfClients,&maxNrOfClients);

            #ifdef DEBUG
            printf("New connection");
            #endif
          }
        }
        else 
        {
          //if not the listener its a regular client

        }

      }
    }

  }

  return(0);
}
