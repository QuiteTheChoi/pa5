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

void openAccount(int sock_desc, char name[100]) {

    if (myBank->numAccounts == 20) {
        fprintf(stderr, "The maximum number of accounts has been reached. You may not open one at this time.");
    }
    
    else {
        for (int i = 0; i < myBank->numAccounts; i++) {
            if (strcmp(myBank->accounts[i].name,name) == 0) {
                fprintf(stderr, "There exists an account with the name that you have given. You may not open one at this time.");
            }
        }
        
        strcpy(myBank->accounts[myBank->numAccounts].name, name);
        myBank->accounts[myBank->numAccounts].session = 1;
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
            
            for (int i = 0; i < 20; i++) {
                myBank->accounts[i].session = 0;
            }
            
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

        //char reply[500];

        //scanf("%s\n", reply);

        //fgets(reply, 500, stdin);

        //gets(reply);

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

            
            while(1){
                int clientsd;

                clientsd = accept(sd, (struct sockaddr *)&saddr, &saddrlen);

                int pid = fork();

                if (pid != 0) { /*If this is not the child process*/

        //printf("HEY\n");
                    close(clientsd);

                }

                else {

                    char * message = "Enter \"0\" to open an account.\nEnter \"1\" to start a session.\nEnter \"2\" for credit.\nEnter \"3\" for debit.\nEnter \"4\" for balance.\nEnter \"5\" to finish a session.\nEnter \"6\" to exit.\n";

                    write(clientsd, message, strlen(message));


                    read(clientsd, buffer, sizeof(buffer)-1);
                    //printf("%s\n", buffer);

                    char * reply = "Message received.\n";

        //char reply[500];

        //scanf("%s\n", reply);

        //fgets(reply, 500, stdin);

        //gets(reply);

                    write(clientsd, reply, strlen(reply));
        
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
