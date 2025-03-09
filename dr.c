/*
FILE          : dr.c
PROJECT       : Assignment 3- System Programming
PROGRAMMER    : ANCHITA KAKRIA, UTTAM ARORA, ARYAN MALAVIYA
FIRST VERSION : 8 MAR 2025
DESCRIPTION   : This file implements the function for dr that sets up a message queue
                for communication with dc.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define SHM_KEY 16535  //key for memory
#define MSG_KEY 12345  //key for message queue

struct message { //struct
    long msgType; //type of message
    int dcID; //id of the sender
    char data[100]; //message
};

typedef struct {
    int msgQueueID; //id of message
    int numDCs;  //number of active dc
    int dcList[10]; //list
} MasterList;

int main() {
    int msgQueueID, shmID;
    struct message msg;

    // FUNCTION    : msgget
    // DESCRIPTION : Message queue is created here or gets back to an existing one
    // PARAMETERS  : MSG_KEY
    // RETURNS     : gets messages on a queue or exists if error

    msgQueueID = msgget(MSG_KEY, IPC_CREAT | 0666); //this creates the message queue or access the existing one
    if (msgQueueID == -1) {
        perror("msgget failed");
        exit(1);
    }
    printf("Message queue created with ID: %d\n", msgQueueID);

    // FUNCTION    : shmget
    // DESCRIPTION : creates a memory space
    // PARAMETERS  : SHM_KEY
    // RETURNS     : give shared memory ID or exists if error

    shmID = shmget(SHM_KEY, sizeof(MasterList), IPC_CREAT | 0666); //this creates the memory or access the existing one
    if (shmID == -1) {
        perror("shmget failed");
        exit(1);
    }
    printf("Shared memory created with ID: %d\n", shmID);

    // FUNCTION    : shmat
    // DESCRIPTION : attaches the memory to the process
    // PARAMETERS  : shmID 
    // RETURNS     : pointer to memory

    MasterList* master = (MasterList*)shmat(shmID, NULL, 0); //attaching memory to address
    if (master == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    master->msgQueueID = msgQueueID;
    master->numDCs = 0;

    printf("Connected to message queue. Waiting for a message...\n");

    while (1) {

        ssize_t bytesReceived = msgrcv(msgQueueID, &msg, sizeof(msg) - sizeof(long), 1, 0);

        if (bytesReceived == -1) {
            if (errno == EIDRM) { //if it is deleted externally
                printf("Message queue was deleted. DR shutting down...\n");
                break;
            }
            perror("msgrcv failed");
            exit(1);
        }

        printf("Message received from DC-%d: %s\n", msg.dcID, msg.data); //prints the message that is recieved

        if (master->numDCs < 10) { //updates the memory with active dc
            master->dcList[master->numDCs] = msg.dcID;
            master->numDCs++;
        }

        printf("Updated shared memory: %d active DCs\n", master->numDCs);
    }

    shmdt(master);
    shmctl(shmID, IPC_RMID, NULL);
    msgctl(msgQueueID, IPC_RMID, NULL); //remove

    printf("DR cleanup complete. Exiting...\n"); //cleanup
    return 0;
}
