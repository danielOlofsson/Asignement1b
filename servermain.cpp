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

int initListenerSocket(char* destHost, char* destPort)
{
  int listenSocket;
  int recivedValue;

  struct addrinfo hints, *servinfo, *p;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(destHost, destPort, &hints, &servinfo)) != 0) 
  {
      fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
      exit(1);
  }

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
    exit(1);
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

  struct pollfd *pfds = malloc(sizeof *pfds *maxNrOfClients);

  


  return(0);
}
