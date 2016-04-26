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
  int name_size = 1000;
  char name[name_size];
  char name_chatmate[name_size];

  int sockfd, newsockfd, portno;
  socklen_t clilen;

  int buffer_size = 8;
  char buffer[buffer_size];

  struct sockaddr_in serv_addr, cli_addr;
  //n -> num of characters read/written
  int n;

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

    int isEndOfMessage = 0;

    send(newsockfd, name, name_size - 1, 0);

    recv(newsockfd, name_chatmate, name_size-1, 0);

    while (true) {
      printf("%s: ", name);
      bzero(buffer, buffer_size);
      fgets(buffer, buffer_size-1,stdin);

      if (strcmp(buffer, "@exit\n") == 0)
        exit(0);

      int hasNextMessage = 0;
      if (buffer[0] == '\n') {
        n = recv(newsockfd, buffer, buffer_size-1, MSG_DONTWAIT);
        if (n <= 0)
          printf("%s:\n", name_chatmate);
        else {
          printf("%s: ", name_chatmate);
          do {
            printf("%s", buffer);
//            for (int x = 0; x < buffer_size-1; x++) {
//              if (buffer[x])
//              printf("%c", buffer[x]);
            }
          } while (recv(newsockfd, buffer, buffer_size-1, MSG_DONTWAIT) > 0);
        }
      } else {
        while (true) {
//          printf("sending: %s\n", buffer);
          send(newsockfd, buffer, buffer_size - 1, 0);

          for (int x = 0; x < buffer_size; x++) {
            if (buffer[x] == '\n')
              isEndOfMessage = 1;
          }

          if (isEndOfMessage == 1)
            break;

          fgets(buffer, buffer_size, stdin);
        }

        isEndOfMessage = 0;
      }
    }

    close(newsockfd);
    if (argc == 2)
      close(sockfd);
    return 0;
}
