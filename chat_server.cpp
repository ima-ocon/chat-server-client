/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//when a system call fails
void error(const char *msg)
{
  //prints out the ERROR
    perror(msg);
    //aborts the program
    exit(1);
}

int main(int argc, char *argv[])
{
  //sockfd and newsockfd are file descriptors
  //they store values returned by socket system call and accept system call
//portno -> port num where server accepts connections
     int sockfd, newsockfd, portno;
    //clilen -> size of address of client (needed for accept system call)
     socklen_t clilen;
     //where the chars are read
    int buffer_size = 8;
     char buffer[buffer_size];
     //this struct contains an internet address (address of server, address of client)
     struct sockaddr_in serv_addr, cli_addr;
     //n -> num of characters read/written
     int n;

     //user passes in port number
     //checks if a port is provided
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     //creates a new socket
     //AF_INET -> internet domain
     //2nd argument -> type of socket (here, stream socket)
     //3rd -> protocol; if 0, system chooses
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
        //sets all values in a buffer to 0
        //1st arg -> pointer to buffer (server address)
        //2nd arg -> size of buffer
        //initializes server address to zeros
     bzero((char *) &serv_addr, sizeof(serv_addr));
     //arg -> port number on which server will listen for connections
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     //IP address of machine
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     //port number (conversion performed)
     serv_addr.sin_port = htons(portno);
     //binds the socket to an address
     //args: file descriptor, address, size of address
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
    //process listens in on socket for connections
    //args: socket file descriptor, size of backlog queue (how many connections can wait)
    //2nd arg set to 5 coz maximum handled by most systems
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     //process blocks until a client connects to the server
     //returns a new file descriptor -> should handle all communication on this connection
     //2nd arg -> pointer to address of client, 3rd -> size
     newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);
     if (newsockfd < 0)
          error("ERROR on accept");
    //a client has now connected
    //initializes buffer

    int isEndOfMessage = 0;

    while (true) {
      printf("Server: ");
      bzero(buffer, buffer_size);
      fgets(buffer, buffer_size-1,stdin);

      if (strcmp(buffer, "@exit\n") == 0)
        exit(0);
      if (buffer[0] == '\n') {
        printf("Client: ");
        while (recv(newsockfd, buffer, buffer_size-1, MSG_DONTWAIT) > 0){
          printf("%s",buffer);
        }
  //      printf("end\n");
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
  /*        while (fgets(buffer, buffer_size, stdin) != NULL) {
            send(newsockfd, buffer, buffer_size - 1, 0);
          }
          printf("End of sending\n");*/
  //        printf("end of message\n");
          isEndOfMessage = 0;
      }
      /*bzero(buffer, buffer_size);
      fgets(buffer, buffer_size-1,stdin);

      if (strcmp(buffer, "@exit\n") == 0)
        exit(0);
      if (buffer[0] == '\n') {
        while (recv(newsockfd, buffer, buffer_size-1, MSG_DONTWAIT) > 0){
          printf("%s",buffer);
        }
      }*/
    }

     //reads from the socket
     //blocks until something is read from the socket
     //n = num of chars read


     n = send(newsockfd, "I got your message", 18, 0);
//     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
     close(newsockfd);
     close(sockfd);
     return 0;
}
