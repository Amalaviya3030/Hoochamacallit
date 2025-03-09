#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>

#define MSG_KEY 12345 //message queue key

struct message {
    long msgType; //type of message
    int dcID;   //process id
    char data[100];
};

int main() {
    int msgQueueID; //stores the id of message queue
    struct message msg;

    // FUNCTION    : msgget
    // DESCRIPTION : Cconnects to the message queue
    // PARAMETERS  : MSG_KEY 
    // RETURNS     : gives message queue id otherwise exists if fail.

    msgQueueID = msgget(MSG_KEY, 0666); //tries to connect to the existing message queue
    if (msgQueueID == -1) {
        perror("msgget failed"); //error message
        exit(1);
    }

    printf("Message queue found with ID: %d\n", msgQueueID);
    //message is preparing to be send
    msg.msgType = 1;
    msg.dcID = getpid(); // Send DC process ID
    strcpy(msg.data, "Hello from DC");

    // FUNCTION    : msgsnd
    // DESCRIPTION : Sends a message to the message queue.
    // PARAMETERS  : msgQueueID , msg       
    // RETURNS     : None

    if (msgsnd(msgQueueID, &msg, sizeof(msg) - sizeof(long), 0) == -1) { //sends the message
        perror("msgsnd failed"); //error message
        exit(1);
    }
    //successful here
    printf("Message sent from DC-%d\n", msg.dcID);
    return 0;
}
