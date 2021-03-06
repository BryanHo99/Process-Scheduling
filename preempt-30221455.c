// Name					: Bryan Ho Yung Kynn
// Student ID			: 30221455
// Date started			: 23-9-2019
// Last date modified	: 24-9-2019

#include "process-visualiser.h"		// Library to use functions for process visualising
#include <stdio.h>					// Library to use printf(), sprintf(), fopen(), fscanf() and fclose()
#include <stdlib.h>					// Library to use exit()
#include <unistd.h>					// Library to use sleep()

/* Special enumerated data type for process state */
typedef enum {
	READY, RUNNING, EXIT
} process_state_t;

/* 
 * C data structure used as process control block. The scheduler
 * should create one instance per running process in the system.
 */
typedef struct {
	char process_name[11];	// A string that identifies the process
	
	/* Times measured in seconds */
	int arrivalTime;		// Time process entered system
	int serviceTime;		// The total CPU time required by the process
	int remainingTime;		// Remaining service time until completion
	int totalWaitTime;		// Total time waiting in the ready queue
	int turnaroundTime;		// Total wait time + Service time
	int runningTime;		// Duration of the process currently in running state
	int timeQuantum;		// The time quantum remaining for a process (Time quantum for each process is 2 seconds)
	int row;				// The row index of the process in the visualiser
	
	process_state_t state;	// Current process state (e.g. READY)
} pcb_t;

// Function prototypes
void roundRobin(pcb_t* data, const char* colour[], int numberOfProcesses);

int main(int argc, char *argv[]) {
	const char* colour[10] = {
		"maroon", "red", "purple", "fuchsia", "green",
		"lime", "olive", "yellow", "navy", "teal"
	};
	
	FILE *filePointer;							// Pointer to the file
	pcb_t data[10];								// data array compiles all the processes found in the text file and is then sorted
	pcb_t temp;									// temp is a variable used to temporarily store a pcb_t structure for swapping
												
	int i = 0, j = 0;							// Array indexes for data array
	int numberOfProcesses;						// Stores the total number of processes
	
	// If there is no specific filename included in the command line, read from process-data.txt (default)
	// Else, take the first argument in the command line after ./non-preempt
	if(argc == 1)
		filePointer = fopen("process-data.txt", "r");
	else
		filePointer = fopen(argv[1], "r");
	
	// Terminate program if file not found 
    if (filePointer == NULL) {
      printf("Error! File not found!");
      exit(1);
	}
	
	// Add processes into the data array
	while(fscanf(filePointer, "%s %d %d", data[i].process_name, &data[i].arrivalTime, &data[i].serviceTime) != EOF) {
		data[i].remainingTime = data[i].serviceTime;
		data[i].totalWaitTime = 0;
		data[i].turnaroundTime = 0;
		data[i].runningTime = 0;
		data[i].timeQuantum = 2;				// Set time quantum to 2 seconds for every process
		i++;
	}
	
	// Set total number of processes added
	numberOfProcesses = i;
	
	// Sort the data array in the order of arrival time
	for(i = 0; i < numberOfProcesses; i++) {
		for(j = i + 1; j < numberOfProcesses; j++) {
			if(data[i].arrivalTime > data[j].arrivalTime) {
				temp = data[i];
				data[i] = data[j];
				data[j] = temp;
			}
		}
	}
	
	// Initialise process visualiser
	initInterface("white", "black");
	
	// Execute the Round Robin scheduling algorithm on the simulator
	roundRobin(data, colour, numberOfProcesses);
	
	// Allows the simulator window to stay until the user closes it themselves
	waitExit();
	
	fclose(filePointer);
	exit(0);
}

// Function that performs the Round Robin algorithm on the simulator
void roundRobin(pcb_t* data, const char* colour[], int numberOfProcesses) {
	int i;										// General array index used in looping through processes in an array
	int front = 0, rear = 0;					// Array indexes for queue array
	int time = 0;								// Current time in seconds
	int processesRemaining = numberOfProcesses;	// Current number of processes that needs to be completed respectively
	pcb_t queue[10];							// queue array acts as a ready queue
	pcb_t temp;									// Variable to temporarily store a process to shift it back to the end of the queue
	char caption[30];							// An array to store a caption which is used in appendBar()
	
	// While there are still processes remaining, continue executing in the simulator
	while(processesRemaining > 0) {
		sleep(1);					// Animate the simulator for every 1 second
		
		// Check through all the processes in data if their arrival time == current clock time
		// If so, add that process into the queue and append a row in the simulator
		for(i = 0; i < numberOfProcesses; i++) {
			if(data[i].arrivalTime == time) {
				queue[rear] = data[i];
				queue[rear].row = appendRow(queue[rear].process_name);
				
				// As the new process enters the queue, it is set as READY
				queue[rear].state = READY;
				printf("%s entered the system at %d seconds.</n>", queue[rear].process_name, queue[rear].arrivalTime);
				
				// Add blank bars into a particular row with the length of their arrival time
				appendBlank(queue[rear].row, queue[rear].arrivalTime);
				rear++;
			}
		}
		
		// Set the process at the front of the queue to be RUNNING as it is currently being executed by the processor
		// Append a coloured bar with a length of 1 for every second
		queue[front].state = RUNNING;
		queue[front].runningTime++;
		sprintf(caption, "Running for %d seconds.", queue[front].runningTime);
		appendBar(queue[front].row, 1, colour[queue[front].row], caption, 0);
		
		// As for the rest of the processes in the queue waiting, append a dotted bar 
		// to their rows of length 1 for every second
		for(i = front + 1; i < rear; i++) {
			// Increment each process' waiting time in the queue
			queue[i].totalWaitTime++;
			
			sprintf(caption, "Waiting for %d seconds.", queue[i].totalWaitTime);
			appendBar(queue[i].row, 1, colour[queue[i].row], caption, 1);
		}
		
		// Decrement the remaining time of the running process and time quantum by 1 second
		queue[front].remainingTime--;
		queue[front].timeQuantum--;
		
		// Once the remaining time of the running process is 0, calculate turnaround time, change state to EXIT,
		// print out completion message, delete the process in the front by pushing all the other processes up by 1 index,
		// then decrement the rear and the number of processes remaining
		if(queue[front].remainingTime <= 0) {
			// Turnaround time = Service time + Total wait time
			// Set its state to EXIT
			queue[front].turnaroundTime += queue[front].serviceTime + queue[front].totalWaitTime;
			queue[front].state = EXIT;
			
			// Print out completion message
			printf("%s completed. Turnaround time: %d seconds. Total wait time: %d seconds.</n>",
				queue[front].process_name, queue[front].turnaroundTime, queue[front].totalWaitTime);
			
			// Delete the front process by pushing all the processes behind in front by 1 index and decrement rear
			for(i = 0; i < rear - 1; i++) { 
                queue[i] = queue[i + 1]; 
            }
			
            rear--;
			processesRemaining--;
		}
		// Else if the process' time quantum is 0 and it is the only process it the queue,
		// reset its time quantum back to 2 seconds again
		else if(queue[front].timeQuantum <= 0 && front == rear - 1) {
			queue[front].timeQuantum = 2;
		}
		// Else if the process' time quantum is 0 and it is not the only process in the queue, move it to the end of the queue
		else if(queue[front].timeQuantum <= 0 && front != rear - 1) {
			// Store the front process in a temporary variable
			temp = queue[front];
			
			// Shift all the processes in front
			for(i = 0; i < rear - 1; i++) { 
                queue[i] = queue[i + 1]; 
            }
			
            queue[rear - 1] = temp;				// Move the front process to the back of the queue
            queue[rear - 1].state = READY;		// Change its state to READY
            queue[rear - 1].timeQuantum = 2;	// Reset time quantum back to 2 seconds
            queue[rear - 1].runningTime = 0;	// Reset its running time back to 0
		}
		
		
		// After 1 whole iteration, increment the clock time by 1
		time++;
	}
}
