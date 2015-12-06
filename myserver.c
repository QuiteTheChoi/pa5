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

//struct bank myBank = myBank;
//myBank->numAccounts = 0;

int openAccount(int sock_desc, char name []) {
    if (myBank->numAccounts == 20) {
        return 1;
    }
    
    else {

       /* char * response = "Please enter a name for your account.\n";

        write(sock_desc, response, strlen(response));

        int r = read(sock_desc, name, sizeof(name)-1);

        int eoi = 500-r;

        name[eoi] = '\0';*/

        for (int i = 0; i < myBank->numAccounts; i++) {
            if (strcmp(myBank->accounts[i].name, name) == 0) {
                return 2;
            }
        }       
        
        strcpy(myBank->accounts[myBank->numAccounts].name, name);
        //printf("HEY\n");
        myBank->numAccounts++;
        return 0;
        //printf("HEY\n");
        
    }
}

/*void shmem_create() {

    key_t key;
    int shmid;
    //char * p;
    bank myBank;
    int	size = 4096;
    char message[] = "Look at the guy next to you and wake him up.\n";
    
    if (errno = 0, (key = ftok( ".", 42)) == -1){
        printf("ftok() failed  errno :  %s\n", strerror(errno));
        exit(1);
    }
    else if (errno = 0, (shmid = shmget( key, size, 0666 | IPC_CREAT | IPC_EXCL)) != -1){		// create ok?
        errno = 0;
        //p = (char *)shmat(shmid, 0, 0);
        myBank = (struct bank *)shmat(shmid, 0, 0);
        
        //if (p == (void *)-1){
        if (myBank == (void *)-1){
            printf("shmat() failed  errno :  %s\n", strerror( errno ) );
            exit(1);
        }
        
        else{
            // Successful creation of shared memory segment.  Segment is filled with zeros.
            // Do some interesting initialization.  Something that takes a while.
            sleep(10);
            printf("Process %d puts message in created shared memory segment attached at address %#x.\n", getpid(), p + sizeof(int));
            sprintf(myBank + sizeof(int), "%s", message);
            *p = 1;
            //shmdt(p);
            shmdt(myBank);
        }
    }
    else if (errno = 0, (shmid = shmget( key, 0, 0666 )) != -1){					// find ok?
        errno = 0;
        p = (char *)shmat(shmid, 0, 0 );
        
        if (p == (void *)-1){
            printf("shmat() failed  errno :  %s\n", strerror( errno));
            exit(1);
        }
        
        else{
            // Acquired shared memory segment.  Has to wait until segment is properly initialized by creator.
            // Could spin around until initialization complete.
            while (*p == 0 ){
                printf("\x1b[2;32mFound segment waiting for initialization to complete.\x1b[0m\n");
            }
            printf("Process %d gets message from shared memory segment attached at address %#x.\n", getpid(), p);
            printf("\n%s\n", p + sizeof(int) );
            //shmdt(p);
        }
    }
    else{											// no create, no find
        printf("shmget() failed  errno :  %s\n", strerror(errno));
        exit(1);
    }
    
    printf("normal end.\n");
}*/

int  main (int argc, char ** argv) {

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
        //if ( p == (void *)-1 ){
        if (myBank == (void *)-1){
            printf("shmat() failed  errno :  %s\n", strerror(errno));
            exit(1);
        }
        else{
            // Successful creation of shared memory segment.  Segment is filled with zeros.
            // Do some interesting initialization.  Something that takes a while.
            myBank->numAccounts = 0;
            
            //sleep( 10 );
            /*printf("Process %d puts message in created shared memory segment attached at address %#x.\n",
                   getpid(), p + sizeof(int));
            sprintf( p + sizeof(int), "%s", message );
            *p = 1;
            shmdt( p );*/
            //shmdt(myBank);

            while(1){
                int clientsd;

                clientsd = accept(sd, (struct sockaddr *)&saddr, &saddrlen);

                int pid = fork();

                if (pid != 0) { /*If this is not the child process*/

                    close(clientsd);
                
                }

                else {

                    char * message = "Enter \"0\" to open an account.\nEnter \"1\" to start a session.\nEnter \"2\" for credit.\nEnter \"3\" for debit.\nEnter \"4\" for balance.\nEnter \"5\" to finish a session.\nEnter \"6\" to exit.\n";

                    write(clientsd, message, strlen(message));

                    read(clientsd, buffer, sizeof(buffer)-1);

                    printf("%s\n", buffer);

                    char * reply = "Message received.\n";

                    write(clientsd, reply, strlen(reply));
        
                }

            }
        
        }
    
    }
    
    else if (errno = 0, (shmid = shmget( key, 0, 0666 )) != -1 ){					// find ok?{
        errno = 0;
        //p = (char *)shmat( shmid, 0, 0 );
        myBank = (bank *)shmat(shmid, 0, 0);
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

                int pid = fork();

                if (pid != 0) { /*If this is not the child process*/

        //printf("HEY\n");
                    close(clientsd);

                }

                else {

                    char response[500];

                    /*char * message = "Enter \"open [your name here]\" to open an account.\nEnter \"start [your name here]\" to start a session.\nEnter \"credit [your amount here]\" for credit.\nEnter \"debit [your amount here]\" for debit.\nEnter \"balance\" for your balance.\nEnter \"finish\" to finish a session.\nEnter \"exit\" to exit.\n";

                    write(clientsd, message, strlen(message));*/

                    int r;
                    while ((r = read(clientsd, buffer, sizeof(buffer)-1)) > 0) {

                        char command[500];
                        char nameOrVal[100];
                      
                        
                    sscanf(buffer, "%s %[^\n]", command, nameOrVal);

                    /*int eoi = 500-r;

                    buffer[eoi] = '\0';*/

                    //printf("%s\n", buffer);

                    if (strcmp(command, "open") == 0) {

                        //printf("HEY\n");
                        int result = openAccount(clientsd, nameOrVal);
                        //printf("Finished Opening Account\n");
                        /*strcat(response, nameOrVal);
                        strcat(response, "!\n");
                        printf("Before write.\n");*/
                        /*write(clientsd, "Thank you for opening an account with us, ", strlen("Thank you for opening an account with us, "));
                        write(clientsd, nameOrVal, sizeof(nameOrVal)-1);
                        write(clientsd, "!", sizeof("!")-1);*/

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


                        //printf("After write.\n");

                    }

                    }


                    //char * reply = "Message received.\n";

                    //printf("HEY\n");

                    //write(clientsd, reply, strlen(reply));

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
