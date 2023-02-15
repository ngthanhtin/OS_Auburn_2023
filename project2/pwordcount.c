/*
 * COMP 7500: Project 2: pWordCount: A Pipe-based WordCount Tool 
 * Thanh Tin Nguyen - 904285164 
 * ttn0011@auburn.edu
 * Date: Date: 2/10/2023
 * Auburn University
 */

/*
 * This work is about designing a C program where two processes are communicate via Unix Pipes.
 * Process 1 will load a file and send the data of this file to Process 2.
 * Process 2 will count the number of words and send this back to the Process 1, 
 * Then in Process 1, display this number.

This work used some code from the sample source code provided on Canvas for Project 2.
*/ 

#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include <errno.h>

#include "utils.h"

#define BUFFER_SIZE 30000
#define READ_END 0
#define WRITE_END 1


int main(int argc, char *argv[])
{
	
	if(argc == 1) // Forget filename
	{
		error_checking(1, "no additional message");
		return 0;
	}
	if(argc > 2) //too many input arguments
	{
		error_checking(2, "no additional message");
		return 0;
	}
	if(argc == 2) // valid inputs
	{
		// check file extension name, the extension must be "txt" (text file)
		char* filename, *filename_extension;
		filename = argv[1];
		filename_extension = check_filename_extension(filename);

		if(strcmp(filename_extension, "txt") != 0)
		{
			error_checking(3, "no additional message");
			return 0;
		}

		// Create two pipes for two processes
		char read_msg[BUFFER_SIZE]; // used to read message
		char write_msg[BUFFER_SIZE]; // used to write message
		pid_t pid;
		int fd1[2]; // file descriptors for pipe 1
		int fd2[2]; // file descriptors for pipe 2

		printf("Creating a pipe ...\n");
		// create the 1st pipe to send message from process 1 to process 2
		if (pipe(fd1) == -1) 
		{
			fprintf(stderr,"Failed to create a pipe\n");
			return 1;
		}
		// create the 2nd pipe to send message from process 2 to process 1
		if (pipe(fd2) == -1) 
		{
			fprintf(stderr,"Failed to create a pipe\n");
			return 1;
		}
		
		// then, fork a child process 
		//process 1 which is the parent process, process 2 or other ones is child process
		printf("Forking 2nd (child) process ...\n");
		pid = fork();
		
		if (pid < 0) 
		{
			fprintf(stderr, "Failed to fork\n");
			return 1;
		}

		if (pid > 0) 
		{ 
			// 1st process, which is also the parent process
			// The 1st process will read the content of a file and save into a buffer
			printf("Process 1 is reading file %s now ...\n", filename);
			FILE *file;
			static char content[BUFFER_SIZE];
			int errnum; // stores error number when attempting to open a file
			file = fopen(filename, "r");
			if(file == NULL) // if can not open the file
			{
				errnum = errno;
				// The strerror might be:
                // 1 Operation not permitted
				// 2 No such file or directory
				// 3 No such process
				// 4 Interrupted system call
				error_checking(4, strerror(errnum)); // load file failed
				exit(0);
			}
			else // opens file file successfully, now read the content of the file into a buffer
			{
				fgets(content,BUFFER_SIZE,file);
				fclose(file);
			}
			
			// copy the data into an array which has the size of BUFFER_SIZE
			strcpy(write_msg, content);

			// close the read end of the pipe1
			close(fd1[READ_END]);
			
			// write to a pipe
			printf("Process 1 starts sending data to Process 2 ...\n");
			write(fd1[WRITE_END], write_msg, strlen(write_msg)+1);
			
			// close the write end of the pipe1
			close(fd1[WRITE_END]);
			
			// wait for the 2nd (child) process
			wait(0);
 
			// close the write end of the pipe2
			close(fd2[WRITE_END]);

			// read from the pipe2
			int num_words;
			read(fd2[READ_END], &num_words, sizeof(num_words));
			//print the result
			printf("The total number of words is %d.\n",num_words);
			// close the read end of the pipe2
			close(fd2[READ_END]);
				
		}
		else // when pid == 0
		{ 
			// 2nd process, which is also the child process

			//close the write end of pipe1
			close(fd1[WRITE_END]);
			
			// read from the pipe1
			read(fd1[READ_END], read_msg, BUFFER_SIZE);
			printf("Process 2 finishes receiving data from Process 1 ...\n");	
			// close the write end of the pipe1
			close(fd1[READ_END]);

			
			// count words
			printf("Process 2 is counting words now ...\n");
			int num_words;
			num_words = count_words(read_msg);

			// close the read end of the pipe2
			close(fd2[READ_END]);
			
			// write to the pipe2
			printf("Process 2 is sending the result back to Process 1 ...\n");
			write(fd2[WRITE_END], &num_words, sizeof(num_words));
			
			// close the write end of the pipe2
			close(fd2[WRITE_END]);
		}
	}	
	return 0;
}
