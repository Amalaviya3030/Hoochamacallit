#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define SHM_KEY 16535
#define MSG_KEY 12345

struct message {
    long msgType;
    int dcID;
    char data[100];
};

typedef struct {
    int msgQueueID;
    int numDCs;
    int dcList[10];
} MasterList;

int main() {
    int msgQueueID, shmID;
    struct message msg;

    msgQueueID = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgQueueID == -1) {
        perror("msgget failed");
        exit(1);
    }
    printf("Message queue created with ID: %d\n", msgQueueID);

    shmID = shmget(SHM_KEY, sizeof(MasterList), IPC_CREAT | 0666);
    if (shmID == -1) {
        perror("shmget failed");
        exit(1);
    }
    printf("Shared memory created with ID: %d\n", shmID);

    MasterList* master = (MasterList*)shmat(shmID, NULL, 0);
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
            if (errno == EIDRM) {
                printf("Message queue was deleted. DR shutting down...\n");
                break;
            }
            perror("msgrcv failed");
            exit(1);
        }

        printf("Message received from DC-%d: %s\n", msg.dcID, msg.data);

        if (master->numDCs < 10) {
            master->dcList[master->numDCs] = msg.dcID;
            master->numDCs++;
        }

        printf("Updated shared memory: %d active DCs\n", master->numDCs);
    }

    shmdt(master);
    shmctl(shmID, IPC_RMID, NULL);
    msgctl(msgQueueID, IPC_RMID, NULL);

    printf("DR cleanup complete. Exiting...\n");
    return 0;
}
