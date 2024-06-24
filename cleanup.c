#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>


typedef struct {
    long mtype;
    int arr;
    int dep;
    int pId;
    int pType;
    int n;  //occupied seats for passenger and number of items for cargo
    int tot_wt;
    int rass;
    int isLanding;
    int hasDeparted;
} Plane;


int main() {

	
	key_t mq_key = ftok("cleanup.c", 'A');
    int mq_id = msgget(mq_key, 0666);
    if (mq_key == -1 || mq_id == -1) {
        perror("Error creating/accessing message queue");
        return 1;
    }

    //printf("Message queue ID: %d\n", mq_id); 837
	char ch;
	

    while (1) {
        printf("Do you want the Air Traffic Control System to terminate? (Y for Yes and N for No): ");
        scanf(" %c", &ch);
        
        if (ch == 'Y' || ch == 'y') {
        
	    Plane plane;
	    plane.mtype = 21;
            plane.arr = 99;
            	//int a = 1;
		if (msgsnd(mq_id, &plane , sizeof(Plane) - sizeof(long) , 0) == -1) {
			perror("msgsnd");
			//exit(EXIT_FAILURE);
	   	}
            break;
        } 
        else if (ch == 'N' || ch == 'n') {
            continue;
        } 
        else {
            printf("Invalid choice, please enter N or Y\n");
        }
    }
    
    //msgctl(msg_id, IPC_RMID, NULL);

    return 0;
}

