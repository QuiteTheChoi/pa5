#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int  main () {
    
    struct addrinfo request, *result, *rp;

/*int getaddrinfo(const char * name, const char * service, const struct addrinfo * request, struct addrinfo ** result);*/

    int check, sd;

    request.ai_flags = AI_PASSIVE;
    request.ai_family = AF_INET;
    request.ai_socktype = SOCK_STREAM;
    request.ai_protocol = 0;
    request.ai_addrlen = 0;
    request.ai_addr = NULL;
    request.ai_canonname = NULL;
    request.ai_next = NULL;

    check = getaddrinfo(NULL, "server", &request, &result);

    if (check != 0){
        fprintf(stderr, "ERROR!\n");
    }

    for (rp = result; rp != NULL; rp = rp->AI_NEXT) {        
        sd = socket(request.ai_family, request.ai_socktype, request.ai_protocol);

        if (sd == -1)
            continue;
        
        if (bind(sd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sd);
    }

    if (rp == NULL) {
        fprintf(stderr, "ERROR: Could not bind.\n");
        return 1;
    }

    freeaddrinfo(result);
    
    listen(sd, 100);
    
    //accept(sd, (struct sockaddr *)&senderaddr, &ic);

    return 0;

}
