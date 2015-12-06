#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

void command_input(void * ptr) {

    char buffer[500];

    int sock_desc = *((int *) ptr);

    while(read(sock_desc, buffer, sizeof(buffer)-1) > 0) {
        printf("%s", buffer);
      }

    exit(0);

}

void response_output(void * ptr) {

    char buffer[500];

    int sock_desc = *((int *) ptr);

    printf("Enter \"open [your name here]\" to open an account.\nEnter \"start [your name here]\" to start a session.\nEnter \"credit [your amount here]\" for credit.\nEnter \"debit [your amount here]\" for debit.\nEnter \"balance\" for your balance.\nEnter \"finish\" to finish a session.\nEnter \"exit\" to exit.\n");

   
    while(read(0, buffer, sizeof(buffer)-1) > 0) {
        write(sock_desc, buffer, sizeof(buffer)-1);
        sleep(2);
        printf("Enter \"open [your name here]\" to open an account.\nEnter \"start [your name here]\" to start a session.\nEnter \"credit [your amount here]\" for credit.\nEnter \"debit [your amount here]\" for debit.\nEnter \"balance\" for your balance.\nEnter \"finish\" to finish a session.\nEnter \"exit\" to exit.\n");
    }

    exit(0);

}

int main(int argc, char *argv[]) {
    struct addrinfo request;
    struct addrinfo *result, *rp;
    int * sd;
    int s, j;
    size_t len;
    ssize_t nread;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    memset(&request, 0, sizeof(struct addrinfo));
    request.ai_flags = 0;
    request.ai_family = AF_INET;
    request.ai_socktype = SOCK_STREAM;
    request.ai_protocol = 0;
    request.ai_addrlen = 0;
    request.ai_addr = NULL;
    request.ai_canonname = NULL;
    request.ai_next = NULL;

    if (getaddrinfo(argv[1], argv[2], &request, &result) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        *sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (*sd == -1)
            continue;
        if (connect(*sd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */
        close(*sd);
    }
    
    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(1);
    }
    
    freeaddrinfo(result);           /* No longer needed */

    pthread_t command, response;

    pthread_create(&command, NULL, (void *) command_input, (void *) sd);
    pthread_create(&response, NULL, (void *) response_output, (void *) sd);

    pthread_join(command, NULL);
    pthread_join(response, NULL);
  
    exit(0);
}
