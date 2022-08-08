#include <stdio.h>
#include <signal.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void initializeSharedMem();
void sighandler(int);
void terminatePellet(int);

int shmid;
int row;
int column;

char (*swimMill)[10][10]; // array shared memory 2d array

int main(int argc, char *argv[]) {
    signal(SIGINT, sighandler); // signal processing
    initializeSharedMem(); // initialize a shared memory space

    srand(time(NULL)); // makes use of the computer's internal clock to control the choice of the seed -- basically initializes random number generation

    row = rand() % 9; // generate a random number between 0 and 9 for the row
    column = rand() % 9; // generate a random number between 0 and 9 for the column
    
    if (*swimMill[row][column] == 'f') { // check if fish already at randomly generated (row, column)
        terminatePellet(1);
    } else {
        *swimMill[row][column] = 'p'; // "drop" the pellet
        sleep(1); // sleep for 1 second before moving
        while (row < 9) { // while the pellet is not at the last row
            *swimMill[row][column] = '~'; // replace the current row with an empty tile
            row++; // increment row
            if (*swimMill[row][column] == 'f') { // check if fish is at the (row, column)
                terminatePellet(1);
            }
            *swimMill[row][column] = 'p'; // pellet moves down 1 row in the stream
            sleep(1); // sleep for 1 second before moving
        }
        if (row == 9) {
            *swimMill[row][column] = '~'; // replace the current row with an empty tile
            terminatePellet(0);
        }
    }

}

void initializeSharedMem(void) {
    key_t key = ftok("shmfile", 1); // generate a key_t type IPC key, suitable for use with shmget
    shmid = shmget(key, sizeof(char[10][10]), IPC_CREAT | 0666); // returns identifier of a shared memory segment associated with the key value
    if (shmid < 0) {
        perror("Failed to generate shared memory id");
        exit(1);
    }
    swimMill = (char(*)[10][10])shmat(shmid, NULL, 0); // attached the shared memory segment to the address space of the calling process
}

void terminatePellet(int eaten) {
    printf("\nPellet ID: %d\n", getpid());
    printf("Coordinates: (%d, %d)\n", row, column);
    if (eaten == 1) {
        printf("Pellet Eaten.\n");
    } else {
        printf("Pellet Missed.\n");
    }
    shmdt(swimMill); // detach from shared memory space
}

void sighandler(int signum) {
    printf("Caught signal %d", signum);
    shmdt(swimMill); // detach from shared memory space
    exit(1);
}