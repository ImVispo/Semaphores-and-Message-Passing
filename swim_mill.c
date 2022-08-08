#include <stdio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>

void initializeSharedMem();
void initializeSemaphore();
void printSwimMill();
void terminateProcesses();
void sighandler(int);
void performSemop(int);

pid_t fish;
pid_t pellet;
int shmid;
char (*swimMill)[10][10]; // array shared memory 2d array
FILE *f;

int semid;
struct sembuf sem_op;

int main(int argc, char *argv[]) {
    signal(SIGINT, sighandler); // signal processing
    initializeSharedMem(); // initialize a shared memory space

    f = fopen("output.txt", "w");
    
    for (int i = 0; i < 10; i++) { // fill 2d array 
        for (int j = 0; j < 10; j++) {
            *swimMill[i][j] = '~';
        }
    }

    semid = semget(IPC_PRIVATE, 1, 0600); // create semaphore; nsems must be greater than 0; senflg is least significant 9 bits of argument
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    semctl(semid, 0, SETVAL, 0600); // initialize semaphore; 0th semaphore

    fish = fork(); // fork fish process
    if (fish == -1) {
        perror("Failed to fork fish");
        exit(1);
    } else if (fish == 0) {
        // execv("Fish", argv); // execute fish
        execl("Fish", "Fish", NULL);
        exit(1);
    }
    
    // Timer
    for (int i = 0; i < 30; i ++) {
        performSemop(-1); // decrement semaphore
        pellet = fork(); // fork pellet process
        if (pellet == -1) {
            perror("Failed to fork pellet");
            exit(1);
        } else if (pellet == 0) {
            execl("Pellet", "Pellet", NULL); // execute pellet
            exit(1);
        }
        printSwimMill(); // prints the current stream to the screen
        printf("Timer: %d\n", i); // prints timer
        sleep(1); // sleep for 1 second
        performSemop(1); // increment semaphore
    }

    terminateProcesses(); // terminate child processes
    sleep(5); // sleep for a few seconds to wait for pellets
    exit(0);
}

void initializeSharedMem() {
    key_t key = ftok("shmfile", 1); // generate a key_t type IPC key, suitable for use with shmget
    shmid = shmget(key, sizeof(char[10][10]), IPC_CREAT | 0666); // returns identifier of a shared memory segment associated with the key value
    if (shmid < 0) {
        perror("Failed to generate shared memory id");
        exit(1);
    }
    swimMill = (char(*)[10][10])shmat(shmid, NULL, 0); // attached the shared memory segment to the address space of the calling process
}

void performSemop(int sem_op_val) {
    sem_op.sem_num = 0;
    sem_op.sem_op = sem_op_val; // increment or decrement the semaphore flag value
    sem_op.sem_flg = 0;
    semop(semid, &sem_op, 1); // perform an atomical operation on the 0th semaphore
}

void printSwimMill() {
    printf("\n");
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            printf("%c", *swimMill[i][j]);
            fprintf(f, "%c", *swimMill[i][j]);
        }
        printf("\n");
        fprintf(f, "\n");
    }
    fprintf(f, "\n");
}

void terminateProcesses() {
    kill(fish, SIGUSR1); // kill fish process
    kill(pellet, SIGUSR1); // kill pellet process
    shmdt(swimMill); // free from shared memory
    shmctl(shmid, IPC_RMID, 0);
}

void sighandler(int signum) {
    printf("Caught signal %d, exiting...", signum);
    terminateProcesses(); // terminate child processes
    exit(1);
}