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
