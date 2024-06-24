#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/wait.h>

typedef struct
{
	long mtype;
	int arr;
	int dep;
	int pId;
	int pType;
	int n; // occupied seats for passenger and number of items for cargo
	int tot_wt;
	int rass; // runway assigned
	int isLanding;
	int hasDeparted;
} Plane;

int main()
{

	Plane plane;
	key_t mq_key;
	int mq_id;

	// key for message queue
	if ((mq_key = ftok("cleanup.c", 'A')) == -1)
	{
		perror("ftok");
		exit(EXIT_FAILURE);
	}

	// message queue with mq_id
	if ((mq_id = msgget(mq_key, 0666)) == -1)
	{
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	int xid = 0;
	printf("Enter Plane ID: ");
	scanf("%d", &xid);
	plane.pId = xid;

	printf("Enter Type of plane: ");
	scanf("%d", &plane.pType); // 1 or 0, 1-passenger, 0-cargo

	int final_wt = 0;
	if (plane.pType == 0) // cargo plane
	{
		printf("Enter number of cargo items: ");
		scanf("%d", &plane.n);

		int wt_av;
		printf("Enter average weight of cargo items: ");
		scanf("%d", &wt_av);

		plane.tot_wt += (plane.n * wt_av) + (2 * 75);
		printf("Total weight = %d\n", plane.tot_wt);
	}
	else // passenger plane
	{
		printf("Enter number of occupied seats: ");
		scanf("%d", &plane.n);

		int pipe_arr[10][2]; // 10 passengers and (read and write end) - file descriptor

		printf("Enter Body Weight and Luggage Weight for all %d passengers\n", plane.n);

		for (int i = 0; i < plane.n; i++)
		{
			pid_t pid;
			// int fd[2];
			if (pipe(pipe_arr[i]) == -1)
			{
				perror("pipe Failed");
				continue;
			}

			pid = fork();

			if (pid < 0)
			{
				perror("Fork failed");
				exit(EXIT_FAILURE);
			}

			else if (pid == 0)
			{

				close(pipe_arr[i][0]); // close read end

				int wt_l;
				scanf("%d", &wt_l); // weight of luggage

				int wt_p;
				scanf("%d", &wt_p); // weight of passenger

				printf("pid of child is %d , parent is %d\n", getpid(), getppid());

				int child_wt = wt_l + wt_p; // weight to be given to pipe. doesn't have crew's weight

				write(pipe_arr[i][1], &child_wt, sizeof(child_wt)); // Write passenger+luggage wt to the pipe
				close(pipe_arr[i][1]);								// Close the write end of the pipe after writing
				exit(EXIT_SUCCESS);
			}
		}
		for (int i = 0; i < plane.n; i++)
		{
			// in parent
			// wait(NULL);
			close(pipe_arr[i][1]); // Close the write end of the pipe in the parent
			int wt;
			while (read(pipe_arr[i][0], &wt, sizeof(wt)) > 0)
			{ // Read luggage details from the pipe. Now Plane process knows the weights
				final_wt += wt;
			}
			close(pipe_arr[i][0]); // Close the read end of the pipe after reading
		}

		plane.tot_wt += final_wt + (75 * 7);
		printf("Total weight: %d\n", plane.tot_wt);
	}
	printf("Enter airport number for departure: ");
	scanf("%d", &plane.dep);
	printf("Enter airport number for arrival: ");
	scanf("%d", &plane.arr);

	// Set message type
	plane.mtype = 21;
	plane.rass = -1;

	// Send plane details to ATC. entire Plane struct sent to ATC with all plane details
	if (msgsnd(mq_id, &plane, sizeof(Plane) - sizeof(long), 0) == -1)
	{
		perror("msgsnd");
		exit(EXIT_FAILURE);
	}

	printf("Plane details sent to ATC\n");


	// landing and termination signal from ATC
	Plane landPlane; //msg stored in this struct
	// xid is the message type
	// msgrcv returns number of bytes received 
	if (msgrcv(mq_id, &landPlane, sizeof(landPlane) - sizeof(long), xid, 0) == -1) // A given plane receives message from ATC about successful departure/arrival 
	{
		// it'll wait until msg queue receives a msg from ATC
		printf("Airports Closed , Sorry :-( \nFlight Cancelled !\n");
		exit(0);
	}

	printf("Landing signal received from ATC");
	printf("\nPlane %d has successfully traveled from Airport %d to Airport %d! \n", landPlane.pId, landPlane.dep, landPlane.arr);

	return 0;
}
