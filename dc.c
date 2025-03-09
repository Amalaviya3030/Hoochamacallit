#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>

#define MSG_KEY 12345

struct message {
    long msgType;
    int dcID;
    char data[100];
};

int main() {
    int msgQueueID;
    struct message msg;

    msgQueueID = msgget(MSG_KEY, 0666);
    if (msgQueueID == -1) {
        perror("msgget failed");
        exit(1);
    }