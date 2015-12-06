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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <signal.h>

typedef struct Account{
    pthread_mutex_t accountLock;
    char name[100];
    float balance;
    int session;
} account;

typedef struct Bank{
    pthread_mutex_t bankLock;
    account accounts[20];
    int numAccounts;
} bank;

bank * myBank;

pthread_mutexattr_t mutattrBank;
pthread_mutexattr_t mutattrAcct;
pthread_t bankInfo;

int openAccount(int sock_desc, char name []) {
    
    char message[500];

    while (pthread_mutex_trylock(&myBank->bankLock) != 0) {
        
        //char message[500];
       
        strcpy(message, "The bank due to another client search. Please hold.\n");

        write(sock_desc, message, sizeof(message)-1);
        
        sleep(2);

    }
    
    if (myBank->numAccounts == 20) {
        pthread_mutex_unlock(&myBank->bankLock);
        
        strcpy(message, "The maximum number of accounts has been reached. You may not open one at this time.\n\n");

        write(sock_desc, message, sizeof(message)-1);
        
        return 1;
    }
    
    else {

        int i;

        for (i = 0; i < myBank->numAccounts; i++) {
            if (strcmp(myBank->accounts[i].name, name) == 0) {
                pthread_mutex_unlock(&myBank->bankLock);
                
                strcpy(message, "There exists an account with the name that you have provided. You may not open one at this time.\n\n");

                write(sock_desc, message, sizeof(message)-1);
                
                return 1;
            }
        }       
        
        strcpy(myBank->accounts[i].name, name);
        myBank->accounts[i].balance = 0;
        myBank->accounts[i].session = 0;
        pthread_mutex_init(&myBank->accounts[myBank->numAccounts].accountLock, &mutattrAcct);
        myBank->numAccounts++;
        pthread_mutex_unlock(&myBank->bankLock);

        sprintf(message, "Thank you for opening an account with us, %s!\n\n", name);
        write(sock_desc, message, sizeof(message)-1);

        return 0;       
    }
}

account * startAccount(int sock_desc, char name []) {

    char message[500];

    while (pthread_mutex_trylock(&myBank->bankLock) != 0) {
        
        //char message[500];
       
        strcpy(message, "The bank is currently locked due to another client search. Please hold.\n");

        write(sock_desc, message, sizeof(message)-1);
        
        sleep(2);

    }
    
    int i;

    for (i = 0; i < myBank->numAccounts; i++) {

        if (strcmp(myBank->accounts[i].name, name) == 0) {
            break;
        }

    }

    pthread_mutex_unlock(&myBank->bankLock);

    if (myBank->numAccounts == i) {

        sprintf(message, "There are no accounts with the name \"%s\" currently open. Please try again later.\n", name);

        write(sock_desc, message, sizeof(message)-1);
       
        return NULL;

    }

    else {

        while (pthread_mutex_trylock(&myBank->accounts[i].accountLock) != 0) {
        
        //char message[500];
       
        sprintf(message, "%s's account is already in session. Please hold.\n", name);

        write(sock_desc, message, sizeof(message)-1);
        
        sleep(2);

    }

        myBank->accounts[i].session = 1;
        
        return &myBank->accounts[i];

    }

}

void client_service(int * sock_desc) {

    int sd = *(int *)sock_desc;

    account * tempAccount = NULL;

    //printf("HEY\n");

    char buffer[500];

    char response[500];

    while (read(sd, buffer, sizeof(buffer)-1) > 0) {
//printf("HEY2\n");
        char command[500];
        char nameOrVal[100];
        
        sscanf(buffer, "%s %[^\n]", command, nameOrVal);

        if (strcmp(command, "open") == 0) {

            if (tempAccount != NULL) {

                strcpy(response, "You are already logged in. You may not open an account at this time.\n");

                write(sd, response, sizeof(response)-1);

            }

            else {
            
                openAccount(sd, nameOrVal);

            }
            
            /*if (result == 0) {

                sprintf(response, "Thank you for opening an account with us, %s!\n\n", nameOrVal);

                write(sd, response, sizeof(response)-1);

            }

            else if (result == 1) {

                strcpy(response, "The maximum number of accounts has been reached. You may not open one at this time.\n\n");

                write(sd, response, sizeof(response)-1);

            }
            
            else {

                strcpy(response, "There exists an account with the name that you have given. You may not open one at this time.\n\n");

                write(sd, response, sizeof(response)-1);
            }*/

        }
        
        else if (strcmp(command, "start") == 0) {

            if (tempAccount != NULL) {

                strcpy(response, "You are already logged in. You may not open an account at this time.\n");

                write(sd, response, sizeof(response)-1);

            }

            else {

                tempAccount = startAccount(sd, nameOrVal);

            }

        }
    
    }

}

void printBankInfo() {

    for (int i = 0; i < myBank->numAccounts; i++) {



    }

}

int main (int argc, char ** argv) {

    char buffer[500];

    struct addrinfo request, *result, *rp;

    struct sockaddr_in saddr;

    socklen_t saddrlen;

    int check, sd;

    memset(&request, 0, sizeof(struct addrinfo));

    request.ai_flags = AI_PASSIVE;
    request.ai_family = AF_INET;
    request.ai_socktype = SOCK_STREAM;
    request.ai_protocol = 0;
    request.ai_addrlen = 0;
    request.ai_addr = NULL;
    request.ai_canonname = NULL;
    request.ai_next = NULL;

    check = getaddrinfo(NULL, argv[1], &request, &result);

    if (check != 0){
        fprintf(stderr, "ERROR!\n");
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
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

    listen(sd, 20);

    //shmem_create();

    key_t key;
    int	shmid;
    //char * p;
    int	size = 4096;
    //char message[] = "Look at the guy next to you and wake him up.\n";
    
    if (errno = 0, (key = ftok( ".", 42 )) == -1){
        printf("ftok() failed  errno :  %s\n", strerror(errno));
        exit(1);
    }
    
    else if (errno = 0, (shmid = shmget( key, size, 0666 | IPC_CREAT | IPC_EXCL )) != -1) {		// create ok?{
        
    errno = 0;
        //p = (char *)shmat( shmid, 0, 0 );
        myBank = (bank *)shmat(shmid, 0, 0);
        pthread_mutexattr_init(&mutattrBank);
        pthread_mutexattr_setpshared(&mutattrBank, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&myBank->bankLock, &mutattrBank);
        //if ( p == (void *)-1 ){
        if (myBank == (void *)-1){
            printf( "shmat() failed  errno :  %s\n", strerror(errno));
            exit(1);
        }
        else{

            myBank->numAccounts = 0;
            // Acquired shared memory segment.  Has to wait until segment is properly initialized by creator.
            // Could spin around until initialization complete.
            /*while ( *p == 0 )
            {
                printf( "\x1b[2;32mFound segment waiting for initialization to complete.\x1b[0m\n" );
            }
            printf( "Process %d gets message from shared memory segment attached at address %#x.\n", getpid(), p );
            printf( "\n%s\n", p + sizeof(int) );
            shmdt( p );*/
            //shmdt(myBank);

            int clientsd;
            //while(1){
            while((clientsd = accept(sd, (struct sockaddr *)&saddr, &saddrlen)) != -1){
                //int clientsd;

                //clientsd = accept(sd, (struct sockaddr *)&saddr, &saddrlen);
                

                pthread_create(&bankInfo, NULL, (void *) printBankInfo, NULL);

                pthread_join(bankInfo, NULL);

                int pid = fork();

                if (pid != 0) { /*If this is not the child process*/

        //printf("HEY\n");
                    close(clientsd);

                }

                else {

                    int * sd = (int *)malloc(sizeof(int));

                    *sd = clientsd;

                    client_service(sd);

                    /*char response[500];

                    while ((r = read(clientsd, buffer, sizeof(buffer)-1)) > 0) {

                        char command[500];
                        char nameOrVal[100];
                      
                        
                    sscanf(buffer, "%s %[^\n]", command, nameOrVal);

                    if (strcmp(command, "open") == 0) {

                        int result = openAccount(clientsd, nameOrVal);
                            if (result == 0) {

                        sprintf(response, "Thank you for opening an account with us, %s!\n\n", nameOrVal);

                        write(clientsd, response, sizeof(response)-1);

                        }

                        else if (result == 1) {

                            strcpy(response, "The maximum number of accounts has been reached. You may not open one at this time.\n\n");

                            write(clientsd, response, sizeof(response)-1);

                        }

                        else {

                            strcpy(response, "There exists an account with the name that you have given. You may not open one at this time.\n\n");

                            write(clientsd, response, sizeof(response)-1);

                        }

                        else if (strcmp(command, "start") == 0) {

                            int result = startAccount(clientsd, nameOrVal);

                        }

                    }

                    }*/

                }

            }

        }

    
    }
    
    else if (errno = 0, (shmid = shmget( key, 0, 0666 )) != -1 ){					// find ok?{
        errno = 0;
        //p = (char *)shmat( shmid, 0, 0 );
        myBank = (bank *)shmat(shmid, 0, 0);
        pthread_mutexattr_init(&mutattrBank);
        pthread_mutexattr_setpshared(&mutattrBank, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&myBank->bankLock, &mutattrBank);
        
        pthread_mutexattr_init(&mutattrAcct);
        pthread_mutexattr_setpshared(&mutattrAcct, PTHREAD_PROCESS_SHARED);
        //if ( p == (void *)-1 ){
        if (myBank == (void *)-1){
            printf( "shmat() failed  errno :  %s\n", strerror(errno));
            exit(1);
        }
        else{
            // Acquired shared memory segment.  Has to wait until segment is properly initialized by creator.
            // Could spin around until initialization complete.
            /*while ( *p == 0 )
            {
                printf( "\x1b[2;32mFound segment waiting for initialization to complete.\x1b[0m\n" );
            }
            printf( "Process %d gets message from shared memory segment attached at address %#x.\n", getpid(), p );
            printf( "\n%s\n", p + sizeof(int) );
            shmdt( p );*/
            //shmdt(myBank);

            int clientsd;
            //while(1){
            while((clientsd = accept(sd, (struct sockaddr *)&saddr, &saddrlen)) != -1){
                //int clientsd;

                //clientsd = accept(sd, (struct sockaddr *)&saddr, &saddrlen);

                pthread_create(&bankInfo, NULL, (void *) printBankInfo, NULL);

                pthread_join(bankInfo, NULL);

                int pid = fork();

                if (pid != 0) { /*If this is not the child process*/

        //printf("HEY\n");
                    close(clientsd);

                }

                else {

                    printf("In child process.\n");

                    int * sd = (int *)malloc(sizeof(int));

                    *sd = clientsd;

                    client_service(sd);

                    /*char response[500];
                    
                    int r;
                    while ((r = read(clientsd, buffer, sizeof(buffer)-1)) > 0) {

                        char command[500];
                        char nameOrVal[100];
                      
                        
                    sscanf(buffer, "%s %[^\n]", command, nameOrVal);

                    if (strcmp(command, "open") == 0) {

                        int result = openAccount(clientsd, nameOrVal);

                        if (result == 0) {

                        sprintf(response, "Thank you for opening an account with us, %s!\n\n", nameOrVal);

                        write(clientsd, response, sizeof(response)-1);

                        }

                        else if (result == 1) {

                            strcpy(response, "The maximum number of accounts has been reached. You may not open one at this time.\n\n");

                            write(clientsd, response, sizeof(response)-1);

                        }

                        else {

                            strcpy(response, "There exists an account with the name that you have given. You may not open one at this time.\n\n");

                            write(clientsd, response, sizeof(response)-1);

                        }


                    }

                    }*/


                }

            }

        }

    }

    else{											// no create, no find{
        printf( "shmget() failed  errno :  %s\n", strerror( errno ) );
        exit(1);
    }

    return 0;

}
