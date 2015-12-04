    key_t key;
    int shmid;
    char * p;
    int size = 4096;
    char message[] = "Hi, I'm Paul.\n";

    if (errno = 0, (key = ftok(".", 42)) == -1) {
        printf("ERROR! ftok() failed.\n");
        exit(1);
    }

    else if (errno = 0, (shmid = shmget(key, size, 0666 | IPC_CREAT | IPC_EXCL)) != -1) {
        errno = 0;
        
        p = (char *)shmat(shmid, 0, 0);
        
        if (p == (void *)-1){
            printf("ERROR! shmat() failed.\n");
            exit(1);
        }

        else {
            sleep(10);

            printf("Process %d puts message in create shared memory segment attached at address %#x.\n", getpid(), p + sizeof(int));

            sprintf(p + sizeof(int), "%s", message);
            *p = 1;
            shmdt(p);

        }

    }

    else if (errno = 0, (shmid = shmget(key, 0, 0666)) != -1) {
        errno = 0;
        p = (char *)shmat(shmid, 0, 0);
        
        if (p == (void *)-1) {
            printf("ERROR! shmat() failed.\n");
            exit(1);
        }

        else {
            while (*p == 0) {
                printf("\x1b[2;32mFound segment waiting for initialization to complete.\x1b[0m\n");
            }

            printf("Process %d get message from shared memory segment attached at address %#x.\n", getpid(), p);
            printf("\n%s\n", p + sizeof(int));
            shmdt(p);
        }
    }

    else {
        printf("shmget() failed errno: %s\n", strerror(errno));
        exit(1);
    }

