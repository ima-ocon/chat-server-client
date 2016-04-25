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
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    int buffer_size = 8;
    char buffer[buffer_size];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
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
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
        int isEndOfMessage = 0;

    while (true) {
      printf("Client: ");
      bzero(buffer, buffer_size);
      fgets(buffer, buffer_size-1,stdin);

      if (strcmp(buffer, "@exit\n") == 0)
        exit(0);

      int hasNextMessage = 0;
      if (buffer[0] == '\n') {
        printf("Server: ");
        while (recv(sockfd, buffer, buffer_size-1, MSG_DONTWAIT) > 0){
          printf("%s",buffer);
        }
      } else {
        while (true) {
//          printf("sending: %s\n", buffer);
          send(sockfd, buffer, buffer_size - 1, 0);

          for (int x = 0; x < buffer_size; x++) {
            if (buffer[x] == '\n')
              isEndOfMessage = 1;
          }

          if (isEndOfMessage == 1)
            break;

          fgets(buffer, buffer_size, stdin);
        }
/*        while (fgets(buffer, buffer_size, stdin) != NULL) {
          send(newsockfd, buffer, buffer_size - 1, 0);
        }
        printf("End of sending\n");*/
//        printf("end of message\n");
        isEndOfMessage = 0;
      }

  //    bzero(buffer, buffer_size);
  //    fgets(buffer, buffer_size,stdin);
  //    n = write(sockfd,buffer,strlen(buffer));
    }

/*    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    printf("%lu\n", strlen(buffer));*/
    close(sockfd);
    return 0;
}
