/* fpont 12/99 */
/* pont.net    */
/* udpServer.c */

/* Converted to echo client/server with select() (timeout option). See udpClient.c */
/* 3/30/05 John Schultz */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <time.h>

#define MAX_MSG 1000

int main(int argc, char *argv[]) {

  int portNo, sd, rc, n, cliLen, flags;
  struct sockaddr_in cliAddr, servAddr;
  char msg[MAX_MSG];
  char msgIn[MAX_MSG];
  char msgRead[MAX_MSG];
  char msgReadFinal[1204];
  float randomlyGenerated, randomPassed;

  /* check command line args */
  if(argc<3) {
    printf("usage : %s <portnumber> <lossfrequency> \n", argv[0]);
    exit(1);
  }

  /* socket creation */
  sd=socket(AF_INET, SOCK_DGRAM, 0);
  if(sd<0) {
    printf("%s: cannot open socket \n",argv[0]);
    exit(1);
  }

  /* Get frequency of drop passed variable */
  randomPassed = atof(argv[2]);

  /* bind local server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  portNo = atoi(argv[1]);
  servAddr.sin_port = htons(portNo);
  rc = bind (sd, (struct sockaddr *) &servAddr,sizeof(servAddr));
  if(rc<0) {
    printf("%s: cannot bind port number %d \n",
	   argv[0], portNo);
    exit(1);
  }

  printf("%s: waiting for data on port UDP %u\n",
	   argv[0],portNo);

/* BEGIN jcs 3/30/05 */

  flags = 0;

/* END jcs 3/30/05 */

  /* server infinite loop */
  while(1) {

    /* init buffer */
    memset(msgIn,0x0,MAX_MSG);

    /* receive message */
    cliLen = sizeof(cliAddr);
    n = recvfrom(sd, msgIn, MAX_MSG, flags,
		 (struct sockaddr *) &cliAddr, &cliLen);

    if(n<0) {
      printf("%s: cannot receive data \n",argv[0]);
      continue;
    }

    int fileSize, numberOfPackets, fd, counter = 0;
    size_t lengthread = 0;
    char packetNum[20], packetTotal[20], padding[20];
    //Get the length of the file
    fd = open(msgIn, O_RDONLY);
    if (fd == -1) error ("ERROR opening file");
    fileSize = lseek(fd, 0L, SEEK_END);
    lseek(fd, 0L, SEEK_SET);
    //set the number of total packets that will be sent
    numberOfPackets = fileSize / MAX_MSG;
    numberOfPackets++;
    sprintf(packetTotal, "%d",numberOfPackets);

    //for padding in header of UDP packet
    if(numberOfPackets < 10)
    {
      strcpy(padding, "0");
      strcat(padding, packetTotal);
      strcpy(packetTotal, padding);
    }

    //Read through the file
    while(lengthread = read(fd, msgRead, MAX_MSG)) {
      if(lengthread == -1) error("ERROR reading from file");
      //Counter to keep track of which packet # is being sent
      counter++;
      printf("Sending Seq# %d of %d\n",counter,numberOfPackets);
      sprintf(packetNum, "%d", counter);
      //for padding in header of udp packet for client readability
      if(counter < 10)
      {
        strcpy(padding, "0");
        strcat(padding, packetNum);
        strcpy(packetNum, padding);
      }
      //Putting header on data packet
      strcpy (msgReadFinal, packetNum);
      strcat (msgReadFinal, packetTotal);
      strcat (msgReadFinal, msgRead);
      //Send data to client on a random basis
      randomlyGenerated=(float)rand()/(float)(RAND_MAX);
      if(randomlyGenerated > randomPassed)
      {
        sendto(sd,msgReadFinal,MAX_MSG + 4,flags,(struct sockaddr *)&cliAddr,cliLen);
      }
      /*
       RESTARTTIMER :bool flag = false, timer = false;
       start a timer here
       while(!flag && !timer)
       {
         if recvfrom then flag = true
         check difference in timer, if over our timeout value then timer = true
       }
       if(timer)
       {
        sendto (resend the packet)
        and then goto:RESTARTTIMER
        }
      */
      //Zero out buffers so they are clean for next while
      bzero(msgRead, MAX_MSG);
      bzero(msgReadFinal, MAX_MSG + 4);
    }

    /* close file */
    if(close(fd) == -1) error("ERROR closing file");

  }/* end of server infinite loop */

return 0;

}
