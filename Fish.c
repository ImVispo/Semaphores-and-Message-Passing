#include <stdio.h>
#include <signal.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

void initializeSharedMem();
void sighandler(int);
int findPellet();
void moveFish(int);

int shmid;
int row;
int column;

char (*swimMill)[10][10]; // array shared memory 2d array

int main(int argc, char *argv[]) {
    signal(SIGINT, sighandler); // signal processing
    initializeSharedMem(); // initialize a shared memory space

    row = 9;
    column = 4;
    *swimMill[row][column] = 'f'; // Fish in the lowest middle position
    while(1) {
        sleep(1); // sleep before moving 
        int pelletColumn = findPellet(); // find closest pellet row
        if (column != pelletColumn) {
            moveFish(pelletColumn);
        }
    }

}

int findPellet() {
    int minDistance = 0;
    int pelletColumn = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (*swimMill[i][j] == 'p') { // if (row, column) has a pellet
                double distance = fabs(sqrt(pow((row - i), 2) + pow((column - j), 2))); // apply distance formula to find the distance of the pellet
                if (minDistance == 0 || distance < minDistance) {
                    minDistance = distance;
                    pelletColumn = j;
                }
            }
        }
    }
    return pelletColumn;
}

void moveFish(int pelletColumn) {
    *swimMill[row][column] = '~';
    if (column < pelletColumn) {
        column++;
    } else {
        column--;
    }
    *swimMill[row][column] = 'f';
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

void sighandler(int signum) {
    printf("Caught signal %d", signum);
    shmdt(swimMill); // detach from shared memory space
    exit(1);
}