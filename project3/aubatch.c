
// Project 3 - AUbatch - A batch scheduling system
// Thanh-Tin Nguyen
// 18/03/2023
// The idea of p-thread messages has been taken from Dr. Qin's source code.

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>

#define MAX_JOB 10

//pthread
pthread_t command_thread, executor_thread;

//global variables
int count = 0;
int head =0;
int tail = 0;
int total=0;
int wait_time=0;
int turn_time=0;
int expected_waiting_time = 0;
int policy=0;

/* Lock and condition variables */
pthread_mutex_t cmd_queue_lock;
pthread_cond_t cmd_buf_not_full;
pthread_cond_t cmd_buf_not_empty;

struct jobQueue
{
   int burst_time, priority;
   char arrival_time[30];
   char name[20];
   char exec_time[30];
   char comp_time[30];
   int num;

};
struct jobQueue job[MAX_JOB-1];

//
void *scheduling(); // producer
void *dispatching(); // consumer
void help(); // help a user to use this program
void parse_run_command(char input_cmd[]); // parse the run command of the user input
void fcfs();
void sjf();
void priority();
void evaluate_performance();
int split_time_string(char time[]);

void help()
{
	printf("\nrun <job> <time> <pri>: submit a job named <job>,\n\t\t execution time is <time>,\n\t\t priority is <pri>.");
	printf("\nlist: display the job status.");
	printf("\nfcfs: change the scheduling policy to FCFS.");
	printf("\nsjf: change the scheduling policy to SJF.");
	printf("\npriority: change the scheduling policy to priority.");
	printf("test <benchmark> <policy> <num_of_jobs> <priority_levels>\n\t\t <min_CPU_time> <max_CPU_time>");
	printf("\nquit: exit AUBatch");

}

int split_time_string(char time[])
{
	char t[30] = "";
	char *token = strtok(time, ":");
	int stoi;
	while (token != NULL)
	{
		strcat(t, token);
		token = strtok(NULL, ":");
	}
	sscanf(t, "%d", &stoi);
	return stoi;
}


void parse_run_command(char input_cmd[])
{
	char cmd[50];
	strcpy(cmd,input_cmd);
	char *token = strtok(cmd, " ");
	int j=1;
	while (token != NULL)
	{
		if (j==2)
			sscanf(token, "%s", &job[head].name);
		if (j==3)
			sscanf(token, "%d", &job[head].burst_time);
		if (j==4)
			sscanf(token, "%d", &job[head].priority);
		j++;
		token = strtok(NULL, " ");
	}
	if(j!=5){
		printf("\nWrong command");
	}
	char *policy_name;
	if (policy == 0) 
	{
		policy_name = "FCFS";
	}
	else if (policy == 1) 
	{
		policy_name = "SJF";
	}
	else if (policy == 2) 
	{
		policy_name = "Priority";
	}
	expected_waiting_time = expected_waiting_time + job[head].burst_time;

	printf("Job %s was submitted.\n", job[head].name);
	printf("Total number of jobs in the queue: %d\n", count);
	printf("Expected waiting time: %d seconds\n", expected_waiting_time);
	printf("Scheduling Policy: %s.\n", policy_name);

	time_t T=time(NULL);
	struct tm tm = *localtime(&T);
	sprintf(job[head].arrival_time,"%d:%d:%d",tm.tm_hour, tm.tm_min, tm.tm_sec);
	job[head].num = head+1;
	count++;
	head++;
	total++;

}

void fcfs()
{
	policy = 0; // the policy indicator of fcfs

	int min_burst, min_priority, min_num;
	char min_name[20], min_time[30];
	int i, j;
	for (i = 1; i < count; i++)
	{   
		min_num = job[i].num;
		min_burst = job[i].burst_time;
		min_priority = job[i].priority;
		strcpy(min_name , job[i].name);
		strcpy(min_time ,job[i].arrival_time);
		j = i-1;
		while ((j >= 0) && (job[j].num > min_num))
		{
			job[j+1].num = job[j].num;
			job[j+1].burst_time = job[j].burst_time;
			job[j+1].priority = job[j].priority;
			strcpy(job[j+1].arrival_time ,job[j].arrival_time);
			strcpy(job[j+1].name ,job[j].name);
			j = j-1;
		}
		job[j+1].num = min_num;
		job[j+1].burst_time = min_burst;
		job[j+1].priority = min_priority;
		strcpy(job[j+1].arrival_time ,min_time);
		strcpy(job[j+1].name , min_name);
	}

	printf("\nScheduling policy is switched to FCFS. All the %d waiting jobs have been rescheduled.",count);
}

void sjf()
{
	policy = 1; // the policy indicator of sjf

	int min_burst, min_priority;
	char min_name[20], min_time[30];
	int i, j;
	for (i = 1; i < count; i++)
	{
		min_burst = job[i].burst_time;
		min_priority = job[i].priority;
		strcpy(min_name , job[i].name);
		strcpy(min_time ,job[i].arrival_time);
		j = i-1;
		while ((j >= 0) && (job[j].burst_time > min_burst))
		{
			job[j+1].burst_time = job[j].burst_time;
			job[j+1].priority = job[j].priority;
			strcpy(job[j+1].arrival_time ,job[j].arrival_time);
			strcpy(job[j+1].name ,job[j].name);
			j = j-1;
		}
		job[j+1].burst_time = min_burst;
		job[j+1].priority = min_priority;
		strcpy(job[j+1].arrival_time ,min_time);
		strcpy(job[j+1].name , min_name);
	}

	printf("\nScheduling policy is switched to SJF. All the %d waiting jobs have been rescheduled.",count);

}

void priority()
{
	policy = 3; //the policy indicator of priority

	int max_priority, max_burst_time;
	char max_name[20], max_time[30];
	int i,j;
	   for (i =1 ; i < count; i++)
	   {
		   max_priority = job[i].priority;

		   max_burst_time = job[i].burst_time;
		   strcpy(max_name,job[i].name);
		   strcpy(max_time ,job[i].arrival_time);
		   j = i-1;
		   while ((j >= 0) && (job[j].priority < max_priority))
		   {
			   job[j+1].priority = job[j].priority;
			   job[j+1].burst_time = job[j].burst_time;
			   stpcpy(job[j+1].arrival_time,job[j].arrival_time);
			   strcpy(job[j+1].name , job[j].name);
			   j = j-1;
		   }
		   job[j+1].priority = max_priority;
		   job[j+1].burst_time = max_burst_time;
		   strcpy(job[j+1].arrival_time, max_time);
		   stpcpy(job[j+1].name,max_name);
		}
	printf("\nScheduling policy is switched to Priority. All the %d waiting jobs have been rescheduled.",count);

}

/* Scheduler thread */
void *scheduling()
{
	int i, j;
	float avg_turntime, avg_waittime, throughtput;
	char *input_cmd1;
	char input_cmd[50];
    size_t input_size = 32;
	input_cmd1 = (char *)malloc(input_size * sizeof(char));
	for(i = 0;i < MAX_JOB; i++)
	{
		pthread_mutex_lock(&cmd_queue_lock);
        // printf("\nScheduler: count = %d", count);
        while (count == MAX_JOB) 
		{
			/* Waits until the buffer is not full */
            pthread_cond_wait(&cmd_buf_not_full, &cmd_queue_lock);
        }
		pthread_mutex_unlock(&cmd_queue_lock);
		printf("\n>");
		getline(&input_cmd1,&input_size,stdin);

		strncpy(input_cmd,input_cmd1,strlen(input_cmd1)-1);
		if(strcmp(input_cmd,"help")==0)
		{
			help();
		}

		else if(strstr(input_cmd,"run"))
		{
			pthread_mutex_lock(&cmd_queue_lock);
			parse_run_command(input_cmd);
			if (head == MAX_JOB)
				head = 0;

        	pthread_cond_signal(&cmd_buf_not_empty);

        	pthread_mutex_unlock(&cmd_queue_lock);
		}
		else if(strcmp(input_cmd,"list")==0)
		{
			pthread_mutex_lock(&cmd_queue_lock);
			printf("\nName\tCPU_Time\tPri\tArrival_time\t\tProgress");
			for(j=0;j<count;j++)
			{
				printf("\n%s\t%d\t\t%d\t\t%s\tRun",job[j].name,job[j].burst_time,job[j].priority,job[j].arrival_time);
			}
			pthread_mutex_unlock(&cmd_queue_lock);
		}
		else if(strcmp(input_cmd,"fcfs")==0)
		{
			fcfs();
		}
		else if(strcmp(input_cmd,"sjf")==0)
		{
			pthread_mutex_lock(&cmd_queue_lock);
			sjf();
			pthread_mutex_unlock(&cmd_queue_lock);
		}
		else if(strcmp(input_cmd,"priority")==0)
		{
			pthread_mutex_lock(&cmd_queue_lock);
			priority();
			pthread_mutex_unlock(&cmd_queue_lock);
		}
		else if(strcmp(input_cmd,"quit")==0)
		{
			avg_turntime = turn_time/total;
			avg_waittime = wait_time/total;
			throughtput = 1.0/avg_turntime;
			printf("\nTotal jobs submitted : %d",total);
			printf("\nAverage waiting time : %f",avg_waittime);
			printf("\nAverage turn around time : %f",avg_turntime);
			printf("\nThroughput : %f\n",throughtput);
			exit(0);
		}
		else
		{

			printf("\n Type help to get the list of commands");
		}
		memset(input_cmd,0,sizeof(input_cmd));
	}

	return NULL;
}
/* Dispatcher thread */
void *dispatching()
{

	int i;
	char arg1[20],arg2[20];

	for (i = 0; i <total+1; i++) 
	{
		sleep(20);
        pthread_mutex_lock(&cmd_queue_lock);

        while (count == 0) 
		{
			// Wait until the buffer has atleast one job
            pthread_cond_wait(&cmd_buf_not_empty, &cmd_queue_lock);

        }
        count--;

    	sprintf(arg1,"%s",job[tail].name);
		sprintf(arg2,"%d",job[tail].burst_time);
		time_t T1=time(NULL);
		struct tm tm1 = *localtime(&T1);
		sprintf(job[tail].exec_time,"%d:%d:%d",tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
		pid_t forked = fork();
		if (forked==0)
		{

			execv("./process",(char*[]){"./process",arg1,arg2,NULL});

		}
		time_t T3=time(NULL);
		sleep(2);
		struct tm tm2 = *localtime(&T3);
		sprintf(job[tail].comp_time,"%d:%d:%d",tm2.tm_hour, tm2.tm_min, tm2.tm_sec);


		evaluate_performance(job[tail].arrival_time,job[tail].exec_time,job[tail].comp_time);

        tail++;
        if (tail == MAX_JOB)
		{
			tail = 0;
		}

      	pthread_cond_signal(&cmd_buf_not_full);

      	pthread_mutex_unlock(&cmd_queue_lock);


	}
	return NULL;
}
void evaluate_performance(char arrival_time[],char exec_time[],char comp_time[])
{

	int arr, exec, comp; // arrival time, excecution time and compute time
	arr = split_time_string(arrival_time);
	exec = split_time_string(exec_time);
	comp = split_time_string(comp_time);
	wait_time = abs(wait_time+(exec-arr));
	turn_time = abs(turn_time + wait_time+(comp-exec));
	expected_waiting_time = expected_waiting_time - job[tail].burst_time;

}

int main()
{	
	printf("Welcome to Thanh-Tin Nguyen's batch job scheduler Version 1.0 \nType 'help' to find more about AUBatch commands.");
	
	// create two threads
	int error1, error2;
	char *message1 = "command thread";
	char *message2 = "executor thread";

	error1 = pthread_create(&command_thread,NULL,scheduling, (void*) message1);
	error2 = pthread_create(&executor_thread,NULL,dispatching, (void*) message2);
  	if(error1 != 0)
		printf("\n Cannot create thread : %s",strerror(error1));
 	if(error2 != 0)
		printf("\n Cannot create thread : %s",strerror(error2));

	// initialize the lock and two conditional variables
	pthread_mutex_init(&cmd_queue_lock, NULL);
	pthread_cond_init(&cmd_buf_not_full, NULL);
	pthread_cond_init(&cmd_buf_not_empty, NULL);

	// wait until all threads complete
  	pthread_join(command_thread, NULL);
  	pthread_join(executor_thread, NULL);

	return 0;
}


