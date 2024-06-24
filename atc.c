#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

// Define the structure for Plane
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

	FILE *file1 = fopen("AirTrafficController.txt", "w");
	fclose(file1);
    //837
    int n; // Number of airports
    printf("Enter number of airports to be handled/managed: ");
    scanf("%d", &n);

    // Create or access the message queue
    key_t mq_key = ftok("cleanup.c", 'A');
    int mq_id = msgget(mq_key, IPC_CREAT | 0666);
    if (mq_key == -1 || mq_id == -1) {
        perror("Error creating/accessing message queue");
        return 1;
    }
    
    int noDepart = 0;
   	int openAirports = n; 
    
    while(true){
	    
		Plane msg;
	    memset( &msg , 0 , sizeof(msg));

	    // Receiving details from plane
	    while(msgrcv(mq_id, &msg, sizeof(Plane) - sizeof(long), 21, 0) == -1); //
		
	    if(msg.arr == 99){
	    	noDepart = 1;
	    	printf("\nCleaning up\n");
	    	//send close msg to all airport
	    	for(int i = 1 ; i <= n ; i++){
	    		Plane closePlane;
	    		closePlane.arr = 98;
	    		closePlane.mtype = i + 10;
	    		msgsnd(mq_id , &closePlane , sizeof(Plane) - sizeof(long) , 0);
			}
	    }
	    
	    if(msg.arr == 98){ //msg from airport that it is closing
	    	//printf("cleaning");
	    	openAirports--;
	    	
	   		if(openAirports == 0)
	   			break;
	   	}
	    
		    // Displaying received message for confirmation
		printf("\n Received Message in airtrafficcontroller.c:\n");
		printf("Arrival: %d\n", msg.arr);
		printf("Departure: %d\n", msg.dep);
		printf("Plane ID: %d\n", msg.pId);
		printf("Total Weight: %d\n", msg.tot_wt);
		printf("Message Type: %ld\n", msg.mtype);//show 1to1
		printf("Runway assigned: %d\n", msg.rass);
		    
		Plane j_msg;
		//sending to dep and arrival if msg from plane
        if(msg.rass==-1 && noDepart == 0)
        	{
			    j_msg.pId = msg.pId;
			    j_msg.dep = msg.dep;
			    j_msg.arr = msg.arr;
			    j_msg.pType = msg.pType;
			    j_msg.n = msg.n;
			    j_msg.tot_wt = msg.tot_wt;
			    j_msg.hasDeparted = 0;
			    j_msg.mtype = msg.arr + 10;
			    
			    if (msgsnd(mq_id, &j_msg, sizeof(Plane) - sizeof(long), 0 ) == -1) {
					perror("Error while sending message to arrival airport");
					return 1;
			    }
			    printf("\nMessage sent to arrival airport\n");
			    
			    j_msg.mtype = msg.dep + 10;
			    if (msgsnd(mq_id, &j_msg, sizeof(Plane) - sizeof(long), 0 ) == -1) {
					perror("Error while sending message to departure airport");
					return 1;
		    	}
		    	printf("Message sent to departure airport\n \n");
		    	continue;
		    }
		    
		    //sending to plane if from airports
		else if(msg.rass>=1&&msg.rass<=11)
			{
		    	msg.mtype=msg.pId;
		    	//printf("In atc->plane else\n");
		    	//dep
		    	if(msg.isLanding==0)
		    	{
		    		//flight departed so msg written in file
		    		FILE *file = fopen("AirTrafficController.txt", "a");
		    		fprintf(file, "Plane %d has departed from Airport %d and will land at Airport %d.\n", msg.pId, msg.dep, msg.arr);
		    		    	fclose(file);
		    		printf("Plane %d has departed from Airport %d and will land at Airport %d.\n", msg.pId, msg.dep, msg.arr);
		    		printf("Details appended to file\n");
		    		
		    		j_msg.pId = msg.pId;
				    j_msg.dep = msg.dep;
				    j_msg.arr = msg.arr;
				    j_msg.pType = msg.pType;
				    j_msg.n = msg.n;
				    j_msg.hasDeparted = 1;
				    j_msg.tot_wt = msg.tot_wt;
				    j_msg.mtype = msg.arr + 10;
				    
				    if (msgsnd(mq_id, &j_msg, sizeof(Plane) - sizeof(long), 0 ) == -1) {
						perror("Error while sending message to arrival airport");
						return 1;
				    }
				    printf("Message sent to arrival airport to prepare for arrival of departed aircraft\n");	
		    	}
		    	else
		    	{	
					printf("Sending message to plane to terminate\n");
					j_msg.mtype = msg.pId;
					j_msg.pId = msg.pId;
					j_msg.dep = msg.dep;
					j_msg.arr = msg.arr;
					j_msg.hasDeparted = 0;
						//atc to plane after arrival signal given---for termination
					if (msgsnd(mq_id, &j_msg, sizeof(Plane) - sizeof(long), 0 ) == -1) {
						perror("Error while sending message to plane");
						return 1;
					}
		    	}
		    	continue;
		    }
		}
    	
    	msgctl(mq_id, IPC_RMID, NULL);
    	execlp("ipcrm" ,  "ipcrm" , "-a", NULL);
		
	return 0;
}
