#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


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