#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
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


typedef struct {
    Plane msg;
    int run;
    int runwayWeights[11];
    int id;
} Params;

int planeWaiting = 0;

pthread_mutex_t mutex[11] = PTHREAD_MUTEX_INITIALIZER; // 11 for 11 runways 

void *handle_plane_departure(void *m) {
	
	Params *p1= (Params *)m;
	Plane *msg1 = &(p1->msg); //msg1 is a pointer to the 'Plane msg' parameter in Params
	
	Plane msg = *msg1;
	Params p = *p1;
	
	int mq_id=p.id;
	int diff1=12000;
    int x=0;
    int rass=-1;
                
    for(int i=0;i< p.run;i++) // for each runway best fit criterion is being checked
        { 
            if(msg.tot_wt<=(p.runwayWeights[i])) //plane can land on this runway
                {
                	x=1;
                	int diff = p.runwayWeights[i]-msg.tot_wt;
                	if ( diff< diff1)
                	{
                	       rass=i;
                	       diff1=diff;
                	}
                }
        }
                       
    if(x==0) // backup runway case
        {
            rass = p.run;
        }
                             
    printf ("\nThe runway assigned for plane number %d is runway number - %d\n", (msg.pId), (rass+1));
                
    sleep (3);//boarding
                
    pthread_mutex_lock(&(mutex[rass]));   
    printf("plane number %d is taking off\n", (msg.pId));  
    sleep(2); //take3off
    printf("plane number %d has taken off\n", (msg.pId));

	pthread_mutex_unlock(&(mutex[rass])); //unlock
	printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of airport No. %d \n", msg.pId, (rass+1), msg.dep);

	msg.mtype=21;
	msg.isLanding=0;
	msg.rass=rass+1;
	   
    if (msgsnd(mq_id, &msg, sizeof(Plane) - sizeof(long), 0 ) == -1) {
        	perror("Error while sending message to atc");
        	return NULL;
    	}
    printf("Departure confirmation sent to atc\n");
    pthread_exit(NULL);
                
}

void *handle_plane_arrival(void *m) {
	
	Params *p1= (Params *)m;
	Plane *msg1 = &(p1->msg);
	
	Plane msg = *msg1;
	Params p = *p1;
	
	int mq_id=p.id;
	int diff1=12000;
    int x=0;
    int rass=-1;
    for(int i=0;i< p.run;i++)
        { 
            if(msg.tot_wt<=(p.runwayWeights[i]))
                {
                	x=1;
                	int diff = p.runwayWeights[i]-msg.tot_wt;
                	if ( diff< diff1)
                	{
                	    rass=i;
                	    diff1=diff;
                	}
                }
        }
        
	if(x==0)
        {
            rass = p.run;
        }
                
    printf ("\nThe runway assigned for plane number %d is runway number - %d\n", (msg.pId), (rass+1));
                
    sleep(10); 
    printf("journey of plane ID: %d - 10 seconds \n", (msg.pId));
    sleep(10); 
    printf("journey of plane ID: %d - 20 seconds \n", (msg.pId));
    sleep(10); 
    printf("journey of plane ID: %d - 30 seconds \n", (msg.pId));

    pthread_mutex_lock(&(mutex[rass]));   //lock
    printf("plane ID: %d is landing\n", (msg.pId));  
    sleep(2);//land simulate
    printf("plane ID: %d has landed\n", (msg.pId));  
	pthread_mutex_unlock(&(mutex[rass])); //unlock
	   	
	sleep (3);//deboarding
	printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n", msg.pId, (rass+1),msg.arr);
	   	
	msg.mtype=21;
	msg.isLanding=1;
	msg.rass=rass+1;
	   	
    if (msgsnd(mq_id, &msg, sizeof(Plane) - sizeof(long), 0 ) == -1) {
        	perror("Error while sending message to atc");
        	return NULL;
    	}

    planeWaiting--;
    printf("Landing confirmation sent to atc\n");
                  
    pthread_exit(NULL);
}

pthread_t threadArr[1000];
int threadCount = 0;
int closeKardo = 0;

int main(){

	int n;
	printf("Enter airport number: ");
	scanf("%d", &n);
    int r;

	while(1)
	{
	    	printf("Enter number of runways: ");
	    	scanf("%d", &r);
	    	if(r%2!=0)
	    	{
	    		printf("Number of runways can be an even number only, please enter again\n");
	    		continue;
	    	}
	    	break;
	}

    int runwayWeights[11];

    printf("Enter loadCapacity of runways (given as a space separated list in a single line):\n");

    // Read integers directly into the runwayWeights array
    for (int it = 0; it < r; it++) {
        scanf("%d", &runwayWeights[it]);
    }

    runwayWeights[r]=15000;

	// making unnecessary runway weights as 0
    for (int it = r+1; it < 11; it++) {
        runwayWeights[it]=0;
    }

    // Print the runwayWeights array
    printf("Runway weights:\n");
    for (int i = 0; i <=r; i++) {
        printf("%d ", runwayWeights[i]);
    }
    printf("\n");
    
	int mq_id;
	key_t mq_key;
	if ((mq_key = ftok("cleanup.c", 'A')) == -1) {
		perror("ftok");
		exit(EXIT_FAILURE);
	   }

	if ((mq_id = msgget(mq_key, 0666)) == -1) {
		perror("msgget");
		exit(EXIT_FAILURE);
	   }

    while (true)
        {
			Plane msg;
			//receiving details from atc
			if((msgrcv(mq_id, &msg, sizeof(Plane) - sizeof(long), n + 10 , 0))==-1){
				perror("Error while receiving message from atc");
				return 1;
			}
	
			if(msg.arr == 98){
		
				closeKardo = 1;
		
				if(planeWaiting != 0){
					msgsnd(mq_id , &msg , sizeof(Plane) - sizeof(long) , 0 );
	   				continue;
				}
	   	
	   			for(int i = 0 ; i < threadCount ; i++){
	   				pthread_join((threadArr[i]) , NULL);
	   			}
	   	
	   			msg.mtype = 21;
	   			msgsnd(mq_id , &msg , sizeof(Plane) - sizeof(long) , 0 );
	   	
	   			exit(0);
	   		}
			printf("\nReceived Message in airport.c:\n");
			printf("Arrival: %d\n", msg.arr);
			printf("Departure: %d\n", msg.dep);
			printf("Plane ID: %d\n", msg.pId);
			printf("Total Weight: %d\n", msg.tot_wt);
			printf("Message Type: %ld\n", msg.mtype);
	
			Params p;
		
			p.msg=msg;
			p.run=r;
			p.id=mq_id;
			for(int it1 = 0 ; it1 < 11 ; it1++){
				p.runwayWeights[it1] = runwayWeights[it1];
			}
			
			if(msg.mtype-10 == msg.arr && msg.hasDeparted == 0)
				planeWaiting++;

			if (msg.mtype-10==msg.arr && msg.hasDeparted == 1)
			{
				printf("\nInside arrival thread \n");
				pthread_create(&(threadArr[threadCount++]), NULL, handle_plane_arrival, (void *)&p);
			}
			else if (msg.mtype-10==msg.dep)
			{
				printf("\nInside departure thread \n");
				pthread_create(&(threadArr[threadCount++]), NULL, handle_plane_departure, (void *)&p);                
			}
		}
	return 0;
}
