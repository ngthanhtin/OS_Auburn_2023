/*
 * COMP 7500: Project 2: pWordCount: A Pipe-based WordCount Tool 
 * Thanh Tin Nguyen - 904285164 
 * ttn0011@auburn.edu
 * Date:
 * Auburn University
 */
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>

#include "utils.h"

#define BUFFER_SIZE 30000
#define READ_END 0
#define WRITE_END 1


int main(int argc, char *argv[])
{
	// Check for invalid inputs
	if(argc == 1) // Forget filename
	{
		error_checking(1);
		return 0;
	}
	if(argc > 2) //too many input arguments
	{
		error_checking(2);
		return 0;
	}
	if(argc == 2) // valid inputs
	{
		// check file extension name
		char* filename, *filename_extension;
		filename = argv[1];
		filename_extension = check_filename_extension(filename);

		if(strcmp(filename_extension, "txt") != 0)
		{
			error_checking(3);
			return 0;
		}

		printf("Begin to load file.\n");

		FILE *fp;
        static char res[BUFFER_SIZE];
        fp = fopen(filename,"r");
        if(fp == NULL) // if can not open the file
        {
			error_checking(4); // load file failed
            return 0;
        }
        else // opens file file successfully, now read the content of the file into a buffer
        {
			fgets(res,BUFFER_SIZE,fp);
			fclose(fp);
			printf("Load file into buffer successfully.\n");
        }
		
		char write_msg[BUFFER_SIZE];
		strcpy(write_msg, res);
		
		
		//int child_result=0; //count result in child process
		//int result=0;       //final result
		char read_msg[BUFFER_SIZE];
		pid_t pid;
		//int pid;
		int fd1[2];
		int fd2[2];
		/* create the second pipe to send msg to parent */
		
		printf("Try to create pipe.\n");
		/* create the first pipe to send msg to child */
		//pipe(fd1);
		if (pipe(fd1) == -1) 
		{
			fprintf(stderr,"Pipe failed\n");
			return 1;
		}
		if (pipe(fd2) == -1) 
		{
			fprintf(stderr,"Pipe failed\n");
			return 1;
		}
		
		/* now fork a child process */
		printf("Try to fork a child process\n");
		pid = fork();
		
		if (pid < 0) 
		{
			fprintf(stderr, "Fork failed\n");
			return 1;
		}
		
		if (pid > 0) 
		{ /* parent process */
				close(fd1[READ_END]);
				
				/* write to the pipe */
				printf("Parent process begin to write msg to pipe\n");
				write(fd1[WRITE_END], write_msg, strlen(write_msg)+1);
				
				/* close the write end of the pipe */
				close(fd1[WRITE_END]);
				
				wait(0);
				int result[4];  
				//printf("The result is %d.\n",result[0]);
				/* close the unused end of the pipe */
				close(fd2[WRITE_END]);
				
				/* read from the pipe */
				printf("parent process get result from pipe.\n");
				read(fd2[READ_END], result, sizeof(result));
				//print the result
				printf("The result is %d.\n",result[0]);
				/* close the write end of the pipe */
				close(fd2[READ_END]);
				
				
				printf("process finish.\n");
				
		}
		else 
		{ /* child process */
				close(fd1[WRITE_END]);
				
				/* read from the pipe */
				printf("Child process begin to read msg from pipe\n");
				read(fd1[READ_END], read_msg, BUFFER_SIZE);
				//debug
				//printf("dubug1 %s.\n",read_msg);
				
				
				/* close the write end of the pipe */
				close(fd1[READ_END]);
				//char child_result[32];
					printf("Child begin to count words \"%s\"\n", read_msg);
				int tmp;
				tmp = count_words(read_msg);
				int number[4];
				number[0] = tmp;
				//printf("debug2 %d.\n",number[0]);
				//strcpy(child_result,tmp);
				//printf("debug3 %d.\n",child_result);
				/* close the unused end of the pipe */
				close(fd2[READ_END]);
				//debug
				//printf("dubug2 %s.\n",read_msg);
				
				//unsigned char *tmp = (unsigned char*) child_result;
				//debug
				
				//printf("debug4 %s.\n",tmp);
				/* write to the pipe */
				printf("Child process begin to send result to parent by pipe.\n");
				write(fd2[WRITE_END],number, sizeof(int));
				//printf("debug2 %d.\n",number[0]);
				printf("Child process finish\n");
				/* close the write end of the pipe */
				close(fd2[WRITE_END]);
		}

	}	
	return 0;
}
