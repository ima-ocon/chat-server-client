#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
  //the name of the person
  int name_size = 1000;
  char name[name_size];
  //the name of the chatmate
  char name_chatmate[name_size];

  char socket_check[2];

  int sockfd, newsockfd, portno;
  socklen_t clilen;

  //buffer -> can contain either what the person sends/receives
  int buffer_size = 8;
  char buffer[buffer_size];

  //only for transfer purposes
  char bufferTemp[buffer_size];

  struct sockaddr_in serv_addr, cli_addr;
  //n -> num of characters read/written
  int n;

  //server/client connects

  //if server
  if (argc == 3) {
    strncpy(name, argv[2], 1000);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
       error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
             error("ERROR on binding");

    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd,
                (struct sockaddr *) &cli_addr,
                &clilen);
    if (newsockfd < 0)
         error("ERROR on accept");
  }
  //if client
  else if (argc == 4) {
      strncpy(name, argv[3], 1000);

      portno = atoi(argv[2]);
      newsockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (newsockfd < 0)
          error("ERROR opening socket");

      struct hostent *server;
      server = gethostbyname(argv[1]);
      if (server == NULL) {
          fprintf(stderr,"ERROR, no such host\n");
          exit(0);
      }
      bzero((char *) &serv_addr, sizeof(serv_addr));
      serv_addr.sin_family = AF_INET;
      bcopy((char *)server->h_addr,
           (char *)&serv_addr.sin_addr.s_addr,
           server->h_length);
      serv_addr.sin_port = htons(portno);
      if (connect(newsockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
          error("ERROR connecting");
  }
  else {
     fprintf(stderr,"usage %s hostname port\n", argv[0]);
     exit(0);
  }

  //server/client receives message

//technically a boolean
//
    int isEndOfMessage = 0;

    send(newsockfd, name, name_size - 1, 0);

    recv(newsockfd, name_chatmate, name_size-1, 0);

    int x = 0;

    while (true) {
      printf("%s: ", name);
      bzero(buffer, buffer_size);
      fgets(buffer, buffer_size-1,stdin);

      //hasNextMessage is a boolean (int because C has no booleans)
      //used when sending
      //refer to the sending conditional for more info
      int hasNextMessage = 0;
      //if the user presses enter, receive messages
      if (buffer[0] == '\n') {
        //n -> number of bytes received
        //Usually recv "blocks"--waits indefinitely for an input
        //But with the flag MSG_DONTWAIT, it checks right away if there's
        //an input, assigns 0 if none, then moves on
        n = recv(newsockfd, buffer, buffer_size-1, MSG_DONTWAIT);
        //n < 0 if there's no new response from the other side
        if (n < 0)
          printf("%s:\n", name_chatmate);
        //if there are messages from the other person
        else if (n == 0) {
          close(newsockfd);
          if (argc == 2)
            close(sockfd);
          exit(1);
        }
        else {
          printf("%s: ", name_chatmate);
          do {
            //assigns the received message to bufferTemp
            n = recv(newsockfd, bufferTemp, buffer_size-1, MSG_DONTWAIT);

            if (strcmp(buffer, "@exit\n") == 0) {
              close(newsockfd);
              if (argc == 2)
                close(sockfd);
              exit(0);
            }

            //just puts the name of the chatmate in front of each new line
            for (int x = 0; x < buffer_size-1; x++) {
              printf("%c", buffer[x]);
              if (buffer[x] == '\n' && n > 0)
                printf("%s: ", name_chatmate);
            }

            //puts bufferTemp in buffer
            strncpy(buffer, bufferTemp, buffer_size);
          } while (n > 0);
        }
      } else {
        while (true) {
          //already takes care of overflow
          //keeps on sending until input is consumed
          n = send(newsockfd, buffer, buffer_size - 1, 0);
          printf("%d\n", n);
          if (n == 0) {
            close(newsockfd);
            if (argc == 2)
              close(sockfd);
            exit(0);
          }

          for (int x = 0; x < buffer_size; x++) {
            if (buffer[x] == '\n')
              isEndOfMessage = 1;
          }

          if (isEndOfMessage == 1)
            break;

          fgets(buffer, buffer_size, stdin);
        }

        if (strcmp(buffer, "@exit\n") == 0) {
          close(newsockfd);
          if (argc == 2)
            close(sockfd);
          exit(0);
        }

        isEndOfMessage = 0;
      }
    }

    close(newsockfd);
    if (argc == 2)
      close(sockfd);
    return 0;
}
