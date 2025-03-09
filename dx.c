#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>

#define SHM_KEY 16535 //key for shared memory
#define MSG_KEY 12345 //key for message queue

typedef struct { //struct
    int msgQueueID; //id of message queue
    int numDCs; //no. of active dcs
    int dcList[10]; //list of them
} MasterList;

// FUNCTION    : log_event
// DESCRIPTION : This logs events to both the log file and console
// PARAMETERS  : message 
// RETURNS     : None

void log_event(const char *message) {
    printf("[DX] %s\n", message); //printing to console
    FILE *logFile = fopen("/tmp/dataCorruptor.log", "a"); //appending the log message to the file 
    if (logFile) {
        fprintf(logFile, "%s\n", message);
        fclose(logFile);
    }
}

int main() {
    log_event("DX started. Trying to attach to shared memory...");

    // FUNCTION    : shmget
    // DESCRIPTION : goes and attaches to the shared memory created by dr
    // PARAMETERS  : SHM_KEY 
    // RETURNS     : returns the shared memory id or exists if error

    // Attach to shared memory
    int shmID = shmget(SHM_KEY, sizeof(MasterList), 0666);
    if (shmID == -1) {
        perror("shmget failed (is DR running?)");
        exit(1);
    }
    log_event("Shared memory found. Attaching...");

    // FUNCTION    : shmat
    // DESCRIPTION : integrates the shared memory into dx
    // PARAMETERS  : shmID 
    // RETURNS     : Pointer to the shared memory 

    MasterList *master = (MasterList *)shmat(shmID, NULL, 0);
    if (master == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    log_event("Successfully attached to shared memory.");

    while (1) {
        sleep(5);
        log_event("Checking for available actions...");

        if (master->numDCs == 0) { //if there are no active dcs it will close
            log_event("No active DCs found. DX exiting...");
            break;
        }

        log_event("Selecting a random action...");
        int action = rand() % 2;  // 0 = kill DC, 1 = delete queue

        if (action == 0 && master->numDCs > 0) {

            // FUNCTION    : kill
            // DESCRIPTION : sends the single to stop the dc
            // PARAMETERS  : process id of the random dc
            // RETURNS     : None

            int targetDC = master->dcList[rand() % master->numDCs];
            if (targetDC > 0) {
                log_event("Sending SIGTERM to a DC process.");
                kill(targetDC, SIGTERM);
            }
        }
        else if (action == 1) {

            // FUNCTION    : msgctl
            // DESCRIPTION : deletes the message queue
            // PARAMETERS  : msgQueueID
            // RETURNS     : None

            log_event("Deleting message queue...");
            msgctl(master->msgQueueID, IPC_RMID, NULL);
            break;
        }
    }

    log_event("DX Detaching from shared memory...");
    shmdt(master);
    log_event("DX Exiting.");
    return 0;
}
