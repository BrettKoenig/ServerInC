/* fpont 12/99 */
/* pont.net    */
/* udpClient.c */

/* Converted to echo client/server with select() (timeout option) */
/* 3/30/05 John Schultz */

#include <stdlib.h> /* for exit() */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h> /* memset() */
#include <sys/time.h> /* select() */

#define HEADER_FIELD_LENGTH 2
#define MAX_MSG 1000

/* BEGIN jcs 3/30/05 */

#define SOCKET_ERROR -1

int isReadable(int sd,int * error,int timeOut) { // milliseconds
  fd_set socketReadSet;
  FD_ZERO(&socketReadSet);
  FD_SET(sd,&socketReadSet);
  struct timeval tv;
  if (timeOut) {
    tv.tv_sec  = timeOut / 1000;
    tv.tv_usec = (timeOut % 1000) * 1000;
  } else {
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
  } // if
  if (select(sd+1,&socketReadSet,0,0,&tv) == SOCKET_ERROR) {
    *error = 1;
    return 0;
  } // if
  *error = 0;
  return FD_ISSET(sd,&socketReadSet) != 0;
} /* isReadable */

/* gets the next part of the message header assuming each part of the header is
   2 bytes long */
int gethdr(char * msg) {
  int i;
  char hdr[HEADER_FIELD_LENGTH + 1];
  for (i = 0; i < HEADER_FIELD_LENGTH; i++) {
    hdr[i] = *msg++;
  }
  hdr[HEADER_FIELD_LENGTH + 1] = '\0';
  return atoi(hdr);
}

/* END jcs 3/30/05 */

int main(int argc, char *argv[]) {

  int serverPort, sd, rc, n, echoLen, flags, error, timeOut, fd, seqnum, totalseq, rcvdNumber;
  struct sockaddr_in cliAddr, remoteServAddr, echoServAddr;
  struct hostent *h;
  char msg[MAX_MSG], *msgptr;


  /* check command line args */
  if(argc<4) {
    printf("usage : %s <sender_hostname> <sender_portnumber> <filename> \n", argv[0]);
    exit(1);
  }

  /* get server IP address (no check if input is IP address or DNS name */
  h = gethostbyname(argv[1]);
  if(h==NULL) {
    printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    exit(1);
  }

  printf("%s: sending data to '%s' (IP : %s) \n", argv[0], h->h_name,
   inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));

  remoteServAddr.sin_family = h->h_addrtype;
  memcpy((char *) &remoteServAddr.sin_addr.s_addr,
   h->h_addr_list[0], h->h_length);
  serverPort = atoi(argv[2]);
  remoteServAddr.sin_port = htons(serverPort);

  /* socket creation */
  sd = socket(AF_INET,SOCK_DGRAM,0);
  if(sd<0) {
    printf("%s: cannot open socket \n",argv[0]);
    exit(1);
  }

  /* bind any port */
  cliAddr.sin_family = AF_INET;
  cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  cliAddr.sin_port = htons(0);

  rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
  if(rc<0) {
    printf("%s: cannot bind port\n", argv[0]);
    exit(1);
  }

  flags = 0;
  timeOut = 100; // ms

  /* send data */
  rc = sendto(sd, argv[3], strlen(argv[3])+1, flags,
  (struct sockaddr *) &remoteServAddr,
  sizeof(remoteServAddr));

  if(rc<0) {
    printf("%s: cannot send data \n",argv[0]);
    close(sd);
    exit(1);
  }

  /* open file for writing */
  fd = open(argv[3], O_WRONLY | O_CREAT);

  /* retrieve and process packets */
  rcvdNumber = 0;
  while(1) {
    /* init buffer */
    memset(msg,0x0,MAX_MSG);

    while (!isReadable(sd,&error,timeOut)) {
      printf("\n \n %d of %d packets received \n", rcvdNumber, totalseq);
      exit(1);
    }
    printf("\n");

    /* receive echoed message */
    echoLen = sizeof(echoServAddr);
    n = recvfrom(sd, msg, MAX_MSG + 2 * HEADER_FIELD_LENGTH, flags,
      (struct sockaddr *) &echoServAddr, &echoLen);
    rcvdNumber++;

    if(n<0) {
      printf("%s: cannot receive data \n",argv[0]);
    }

    /* process message header */
    msgptr = msg;
    seqnum = gethdr(msgptr);
    msgptr += HEADER_FIELD_LENGTH;
    totalseq = gethdr(msgptr);
    msgptr += HEADER_FIELD_LENGTH;

    /* print received message */
    printf("%s: Received Seq# %d : %d of %d from %s:UDP%u",
      argv[0],(seqnum+1)%2,seqnum,totalseq,inet_ntoa(echoServAddr.sin_addr),
      ntohs(echoServAddr.sin_port));

    /* print message to file */
    write(fd, msgptr, strlen(msgptr));

   //use SendTo here for ACK

    /* halt if all packets received */
    if (seqnum == totalseq) {
      printf("\n \n %d of %d packets received \n", rcvdNumber, totalseq);
      break;
    }
  }

  /* close file */
  close(fd);

  return 1;

}
