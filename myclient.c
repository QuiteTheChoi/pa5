#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

void command_input(void * ptr) {

    char buffer[500];

    int sock_desc = *((int *) ptr);

    int check;

    while((check = read(sock_desc, buffer, sizeof(buffer)-1)) > 0) {
        printf("%s", buffer);
    }

    if (check == 0) {
        printf("Cannot read from server. Connection closed. Exiting program.\n");
        exit(0);
    }

}

void response_output(void * ptr) {

    char buffer[500];

    int sock_desc = *((int *) ptr);

    int check;

    printf("Enter \"open [your name here]\" to open an account.\nEnter \"start [your name here]\" to start a session.\nEnter \"credit [your amount here]\" for credit.\nEnter \"debit [your amount here]\" for debit.\nEnter \"balance\" for your balance.\nEnter \"finish\" to finish a session.\nEnter \"exit\" to exit.\n");

    while((check = read(0, buffer, sizeof(buffer)-1)) > 0) {
        /*printf("Enter \"open [your name here]\" to open an account.\nEnter \"start [your name here]\" to start a session.\nEnter \"credit [your amount here]\" for credit.\nEnter \"debit [your amount here]\" for debit.\nEnter \"balance\" for your balance.\nEnter \"finish\" to finish a session.\nEnter \"exit\" to exit.\n");*/

       int n= write(sock_desc, buffer, sizeof(buffer)-1);
       if(n ==0){
        printf("Cannot write to server. Connection closed. Exiting program.\n");
        exit(0);
       }
       sleep(2);
       printf("Enter \"open [your name here]\" to open an account.\nEnter \"start [your name here]\" to start a session.\nEnter \"credit [your amount here]\" for credit.\nEnter \"debit [your amount here]\" for debit.\nEnter \"balance\" for your balance.\nEnter \"finish\" to finish a session.\nEnter \"exit\" to exit.\n");
    }

}

int main(int argc, char *argv[]) {
    struct addrinfo request;
    struct addrinfo *result, *rp;
    int  sd;
    int s, j;
    size_t len;
    ssize_t nread;

    struct sockaddr_in dest;
    struct hostent * server;
    socklen_t saddrlen;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s host\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    /*memset(&request, 0, sizeof(struct addrinfo));
    request.ai_flags = 0;
    request.ai_family = AF_INET;
    request.ai_socktype = SOCK_STREAM;
    request.ai_protocol = 0;
    request.ai_addrlen = 0;
    request.ai_addr = NULL;
    request.ai_canonname = NULL;
    request.ai_next = NULL;*/

    //int sockConnection;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Socket error.\n");
        exit(1);
    }

    server = gethostbyname(argv[1]);
    
    if (server == NULL) {
        fprintf(stderr, "The host that you have given does not exist.\n");
        exit(1);
    }

    int portnum = 7771;

    bzero((char *)&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&dest.sin_addr.s_addr, server->h_length);
    dest.sin_port = htons(portnum);
    //dest.sin_addr.s_addr = INADDR_ANY;

    /*if (inet_aton(argv[1], &dest.sin_addr.s_addr) == 0) {
        fprintf(stderr, "Address error.\n");
        exit(1);
    }*/

        /*---Connect to server---*/
    if (connect(sd, (struct sockaddr*)&dest, sizeof(dest)) != 0 ) {
        printf("%s\n", strerror(errno));
        fprintf(stderr, "Connection error.\n");
        close(sd);
        exit(1);
    }

    printf("Connection to bank has been established.\n");

    /*if (getaddrinfo(argv[1], "35000", &request, &result) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        *sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (*sd == -1)
            continue;
        if (connect(*sd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;*/                  /* Success */
        //close(*sd);
    //}
    
    //if (rp == NULL) {               /* No address succeeded */
        /*fprintf(stderr, "Could not connect\n");
        exit(1);
    }
    
    freeaddrinfo(result);*/           /* No longer needed */

    pthread_t command, response;

    pthread_create(&command, NULL, (void *) command_input, (void *) &sd);
    pthread_create(&response, NULL, (void *) response_output, (void *) &sd);

    pthread_join(command, NULL);
    pthread_join(response, NULL);
  
    exit(0);
}
