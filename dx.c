#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>

#define SHM_KEY 16535
#define MSG_KEY 12345

typedef struct {
    int msgQueueID;
    int numDCs;
    int dcList[10];
} MasterList;

void log_event(const char* message) {
    printf("[DX] %s\n", message);
    FILE* logFile = fopen("/tmp/dataCorruptor.log", "a");
    if (logFile) {
        fprintf(logFile, "%s\n", message);
        fclose(logFile);
    }
}

int main() {
    log_event("DX started. Trying to attach to shared memory...");

    int shmID = shmget(SHM_KEY, sizeof(MasterList), 0666);
    if (shmID == -1) {
        perror("shmget failed (is DR running?)");
        exit(1);
    }

    log_event("Shared memory found. Attaching...");
    MasterList* master = (MasterList*)shmat(shmID, NULL, 0);
    if (master == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }

    log_event("Successfully attached to shared memory.");