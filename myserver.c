#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
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
    
    int count = 1;
    while (pthread_mutex_trylock(&myBank->bankLock) != 0) {
        
        //char message[500];
        
        if (count > 8) {
            strcpy(message, "The bank is currently overloaded with clients. Please try again at a later time.\n\n");
            return 1;
        }

        sprintf(message, "The bank is busy due to another client search. Please hold...(%d)\n\n", count);

        write(sock_desc, message, sizeof(message)-1);

        count++;

        sleep(2);        

    }
    
    if (strlen(name) == 0) {
        strcpy(message, "You have not entered a name.\n\n");
        write(sock_desc, message, sizeof(message)-1);
        pthread_mutex_unlock(&myBank->bankLock);
        return 1;
    }

    if (myBank->numAccounts == 20) {        
        strcpy(message, "The maximum number of accounts has been reached. You may not open one at this time.\n\n");

        write(sock_desc, message, sizeof(message)-1);
        
        pthread_mutex_unlock(&myBank->bankLock);
        
        return 1;
    }
    
    else {

        int i;

        for (i = 0; i < myBank->numAccounts; i++) {
            if (strcmp(myBank->accounts[i].name, name) == 0) {
                
                strcpy(message, "There exists an account with the name that you have provided. You may not open one at this time.\n\n");

                write(sock_desc, message, sizeof(message)-1);

                pthread_mutex_unlock(&myBank->bankLock);

                return 1;
            }
        }       
        
        strcpy(myBank->accounts[i].name, name);
        myBank->accounts[i].balance = 0;
        myBank->accounts[i].session = 0;
        pthread_mutex_init(&myBank->accounts[myBank->numAccounts].accountLock, &mutattrAcct);
        myBank->numAccounts++;

        sprintf(message, "Thank you for opening an account with us, %s!\n\n", name);
        write(sock_desc, message, sizeof(message)-1);

        pthread_mutex_unlock(&myBank->bankLock);

        return 0;       
    }
}

account * startAccount(int sock_desc, char name []) {

    char message[500];

    while (pthread_mutex_trylock(&myBank->bankLock) != 0) {
        
        //char message[500];
       
        strcpy(message, "The bank is currently locked due to another client search. Please hold.\n\n");

        write(sock_desc, message, sizeof(message)-1);
        
        sleep(2);

    }

    if (strlen(name) == 0) {
        strcpy(message, "You have not entered a name.\n\n");
        write(sock_desc, message, sizeof(message)-1);
        pthread_mutex_unlock(&myBank->bankLock);
        return NULL;
    }
    
    int i;

    for (i = 0; i < myBank->numAccounts; i++) {

        if (strcmp(myBank->accounts[i].name, name) == 0) {
            break;
        }

    }

    pthread_mutex_unlock(&myBank->bankLock);

    if (myBank->numAccounts == i) {

        sprintf(message, "There are no accounts with the name \"%s\" currently open. Please try again later.\n\n", name);

        write(sock_desc, message, sizeof(message)-1);
       
        return NULL;

    }

    else {

        while (pthread_mutex_trylock(&myBank->accounts[i].accountLock) != 0) {
        
        //char message[500];
       
        sprintf(message, "%s's account is already in session. Please hold.\n\n", name);

        write(sock_desc, message, sizeof(message)-1);
        
        sleep(2);

    }

        myBank->accounts[i].session = 1;

        strcpy(message, "You have successfully started a session.\n\n");
        write(sock_desc, message, sizeof(message)-1);

        return &myBank->accounts[i];
    }

}

void credit(int sock_desc, account * acc, float val) {
    char message[500];
    float round = roundf(val*100)/100;
    acc->balance += round;
    sprintf(message, "You have successfully credited $%.2f to your account.\n\n", val);
    write(sock_desc, message, sizeof(message)-1);
}


void debit(int sock_desc, account * acc, float val) {
    float round = roundf(val*100)/100;

    if (acc->balance < round)
        ;
    else {
        char message[500];
        acc->balance -= round;
        sprintf(message, "You have successfully debited $%.2f from your account.\n\n", val);
        write(sock_desc, message, sizeof(message)-1);
    }
}

void balance(int sock_desc, account * acc) {
    char message[500];
    sprintf(message, "Your account balance is $%.2f.\n\n", acc->balance);
    write(sock_desc, message, sizeof(message)-1);
}

void finishAccount(int sock_desc, account * acc) {
    char message[500];
    acc->session = 0;
    strcpy(message, "You have successfully finished your session.\n\n");
    write(sock_desc, message, sizeof(message)-1);
    pthread_mutex_unlock(&acc->accountLock);
}

void exitSession(int sock_desc, account * acc) {
    char message[500];
    acc->session = 0;
    strcpy(message, "You have exited the bank.\n\n");
    write(sock_desc, message, sizeof(message)-1);
    pthread_mutex_unlock(&acc->accountLock);
}

void client_service(int * sock_desc) {

    int sd = *(int *)sock_desc;

    account * tempAccount = NULL;

    char buffer[500];

    char response[500];

    while (read(sd, buffer, sizeof(buffer)-1) > 0) {
        char command[500];
        char nameOrVal[100];
        
        sscanf(buffer, "%s %[^\n]", command, nameOrVal);

        if (strcmp(command, "open") == 0) {

            if (tempAccount != NULL) {

                strcpy(response, "You are already logged in. You may not start another account.\n\n");

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

                strcpy(response, "You are already logged in. You may not start an account.\n\n");

                write(sd, response, sizeof(response)-1);

            }

            else {

                tempAccount = startAccount(sd, nameOrVal);

            }

        }

        else if (strcmp(command, "credit") == 0) {

            if (strlen(nameOrVal) == 0) {
                strcpy(response, "You have not entered a value.\n\n");
                write(sd, response, sizeof(response)-1);
                continue;
            }

            if (tempAccount == NULL || tempAccount->session == 0) {

                strcpy(response, "You are not logged in. You may not add to credit at this time.\n\n");

                write(sd, response, sizeof(response)-1);

            }

            else {

                float val = atof(nameOrVal);

                if ((fabs(val - 0.0f) < 0.00001) || (val < 0.0f)) {

                    strcpy(response, "The value that you have entered is 0 or invalid.\n\n");

                    write(sd, response, sizeof(response)-1);

                }

                else {

                    credit(sd, tempAccount, val);

                }

            }

        }

        else if (strcmp(command, "debit") == 0) {

            if (strlen(nameOrVal) == 0) {
                strcpy(response, "You have not entered a value.\n\n");
                write(sd, response, sizeof(response)-1);
                continue;
            }

            if (tempAccount == NULL || tempAccount->session == 0) {

                strcpy(response, "You are not logged in. You may not subtract from debit at this time.\n\n");

                write(sd, response, sizeof(response)-1);

            }

            else {
                
                float val = atof(nameOrVal);

                if ((fabs(val - 0.0f) < 0.00001) || (val < 0.0f)) {

                    strcpy(response, "The value that you have entered is 0 or invalid.\n\n");

                    write(sd, response, sizeof(response)-1);

                }

                else {

                    debit(sd, tempAccount, val);

                }

            }

        }

        else if (strcmp(command, "balance") == 0) {

            if (tempAccount == NULL || tempAccount->session == 0) {

                strcpy(response, "You are not logged in. You do not have access to an account balance at this time.\n\n");

                write(sd, response, sizeof(response)-1);

            }

            else {

                balance(sd, tempAccount);

            }

        }

        else if (strcmp(command, "finish") == 0) {

            if (tempAccount == NULL) {

                strcpy(response, "You are not logged in. You cannot finish a session at this time.\n\n");

                write(sd, response, sizeof(response)-1);

            }

            else {

                finishAccount(sd, tempAccount);

            }

        }

        else if (strcmp(command, "exit") == 0) {
            
            if (tempAccount == NULL) {

                strcpy(response, "You have exited the bank.\n\n");

                write(sd, response, sizeof(response)-1);

                close(sd);

                break;

            }

            else {

                exitSession(sd, tempAccount);

                close(sd);

                break;

            }

        }

        else {

            strcpy(response, "The command that you have entered is not valid. Please try again.\n\n");

                write(sd, response, sizeof(response)-1);

        }
    
    }

}

void printBankInfo() {

    while (pthread_mutex_trylock(&myBank->bankLock) != 0) {
        
        //char message[500];
   
        printf("The bank is currently busy. Please hold.\n\n");

        sleep(2);

    }

    while(1){

        int i;

        for (i = 0; i < myBank->numAccounts; i++) {
            printf("Account name: %s\n", myBank->accounts[i].name);
            printf("Balance: $%.2f\n", myBank->accounts[i].balance);
            if (myBank->accounts[i].session == 1)
                printf("IN SERVICE\n");
            else
                ;
            printf("\n");
        }

        pthread_mutex_unlock(&myBank->bankLock);

        sleep(20);
    }

}

static void signal_handler(int signo) {

    if (signo == SIGCHLD) {

        while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {

        }
    }
}

int main (int argc, char ** argv) {

    char buffer[500];

    //struct addrinfo request, *result, *rp;

    struct sockaddr_in saddr;

    socklen_t saddrlen;

    int check, sd;

    struct sigaction action;

    action.sa_flags = 0;
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    //sigaction(SIGINT, &action, 0);
    //sigemptyset(&action.sa_mask);
    sigaction(SIGCHLD, &action, 0);

    /*memset(&request, 0, sizeof(struct addrinfo));

    request.ai_flags = AI_PASSIVE;
    request.ai_family = AF_INET;
    request.ai_socktype = SOCK_STREAM;
    request.ai_protocol = 0;
    request.ai_addrlen = 0;
    request.ai_addr = NULL;
    request.ai_canonname = NULL;
    request.ai_next = NULL;*/

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Socket error.\n");
        exit(1);
    }

    int portnum = 7771;

    bzero((char *)&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(portnum);
    saddr.sin_addr.s_addr = INADDR_ANY;

    //check = getaddrinfo(NULL, "35000", &request, &result);

    /*if (check != 0){
        fprintf(stderr, "Error with getaddrinfo.\n");
    }*/

    /*for (rp = result; rp != NULL; rp = rp->ai_next) {
        sd = socket(request.ai_family, request.ai_socktype, request.ai_protocol);

        if (sd == -1)
            continue;*/

    if (bind(sd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        printf("%s\n", strerror(errno));
        fprintf(stderr, "socket--bind error.\n");
        exit(1);
    }

        /*---Make it a "listening socket"---*/
    if (listen(sd, 20) < 0) {
        fprintf(stderr, "socket--listen error.\n");
        exit(1);
    }


    /*if (bind(sd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sd);
    }

    if (rp == NULL) {
        fprintf(stderr, "ERROR: Could not bind.\n");
        return 1;
    }

    freeaddrinfo(result);

    listen(sd, 20);*/

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

                //pthread_join(bankInfo, NULL);

                int pid = fork();

                if (pid != 0) { /*If this is not the child process*/

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
        
        myBank = (bank *)shmat(shmid, 0, 0);
        pthread_mutexattr_init(&mutattrBank);
        pthread_mutexattr_setpshared(&mutattrBank, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&myBank->bankLock, &mutattrBank);
        
        pthread_mutexattr_init(&mutattrAcct);
        pthread_mutexattr_setpshared(&mutattrAcct, PTHREAD_PROCESS_SHARED);
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

                //pthread_join(bankInfo, NULL);

                int pid = fork();

                if (pid != 0) { /*If this is not the child process*/

                    close(clientsd);

                }

                else {

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
