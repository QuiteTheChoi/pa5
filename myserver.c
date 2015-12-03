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

/*struct addrinfo {
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    size_t ai_addrlen;
    struct sockaddr * ai_addr;
    char * ai_canonname;
    struct addrinfo * ai_next;
};*/
int  main () {
struct addrinfo request, *server;

/*int getaddrinfo(const char * name, const char * service, const struct addrinfo * request, struct addrinfo ** result);*/

int check;
check = getaddrinfo("Hello", "server", &request, &server);

request.ai_flags = AI_PASSIVE;
request.ai_family = AF_INET;
request.ai_socktype = SOCK_STREAM;
request.ai_protocol = 0;
request.ai_addrlen = 0;
request.ai_addr = NULL;
request.ai_canonname = NULL;
request.ai_next = NULL;

}
