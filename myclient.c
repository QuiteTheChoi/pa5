#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define BUF_SIZE 500
int main(int argc, char *argv[]) {
    struct addrinfo request;
    struct addrinfo *result, *rp;
    int sd, s, j;
    size_t len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc < 3) {
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
        sd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sd == -1)
            continue;
        if (connect(sd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */
        close(sd);
    }                                                        }
    
    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }
    
    freeaddrinfo(result);           /* No longer needed */

    for (j = 3; j < argc; j++) {
        len = strlen(argv[j]) + 1;
        /* +1 for terminating null byte */
        if (len + 1 > BUF_SIZE) {
            fprintf(stderr, "Ignoring long message in argument %d\n", j);
            continue;
        }

        if (write(sfd, argv[j], len) != len) {
            fprintf(stderr, "partial/failed write\n");
            exit(EXIT_FAILURE);
        }
        nread = read(sfd, buf, BUF_SIZE);
        if (nread == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        printf("Received %zd bytes: %s\n", nread, buf);
    }
    exit(EXIT_SUCCESS);
}
