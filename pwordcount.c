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
		// create the 1st pipe to send message from proc 1 to proc 2
		if (pipe(fd1) == -1) 
		{
			fprintf(stderr,"Failed to create a pipe\n");
			return 1;
		}
		// create the 2nd pipe to send message from proc 2 to proc 1
		if (pipe(fd2) == -1) 
		{
			fprintf(stderr,"Failed to create a pipe\n");
			return 1;
		}
		
		// then, fork a child process (process 1 which is the parent process, process 2 or other ones is child process)
		printf("Forking a child process ...\n");
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
			printf("Begin to load file.\n");
			FILE *file;
			static char content[BUFFER_SIZE];
			int errnum; // stores error number when attempting to open a file
			file = fopen(filename, "r");
			if(file == NULL) // if can not open the file
			{
				errnum = errno;
				error_checking(4, strerror(errnum)); // load file failed
				exit(0);
			}
			else // opens file file successfully, now read the content of the file into a buffer
			{
				fgets(content,BUFFER_SIZE,file);
				fclose(file);
				printf("Load file into buffer successfully.\n");
			}
			
			// copy the data into an array which has the size of BUFFER_SIZE
			strcpy(write_msg, content);

			// close the read end of the pipe which is an unsued end of the pipe
			close(fd1[READ_END]);
			
			// write to a pipe
			printf("1st (parent) process begins to write message to a pipe\n");
			write(fd1[WRITE_END], write_msg, strlen(write_msg)+1);
			
			// close the write end of the pipe which is an unused end of the pipe
			close(fd1[WRITE_END]);
			
			// wait for the 2nd (child) process
			wait(0);

			int result[4];  
			// close the unused end of the pipe which is an unused end of the pipe
			close(fd2[WRITE_END]);
			
			// read from the pipe 
			printf("1st (parent) process get result from pipe.\n");
			read(fd2[READ_END], result, sizeof(result));
			//print the result
			printf("The result is %d.\n",result[0]);
			// close the write end of the pipe which is an unused end of the pipe
			close(fd2[READ_END]);
			
			printf("process finish.\n");
				
		}
		else // when pid == 0
		{ 
			int parent_status;
			waitpid(pid, &parent_status, 0);
			if WIFEXITED(parent_status)
			{
				printf("hihi");
				exit(0);
			}
			// 2nd process, which is also the child process
			close(fd1[WRITE_END]);
			
			// read from the pipe
			printf("2nd (child) process begins to read message from pipe\n");
			read(fd1[READ_END], read_msg, BUFFER_SIZE);
			
			// close the write end of the pipe which is an unused end of the pipe
			close(fd1[READ_END]);

			printf("2nd (child) process begins to count words \"%s\" \n", read_msg);
			int tmp;
			// count words
			tmp = count_words(read_msg);
			int number[4];
			number[0] = tmp;

			// close the read end of the pipe which is an unused end of the pipe
			close(fd2[READ_END]);
			
			// write to the pipe
			printf("2nd (child) process begins to send result to 1st (parent) process by pipe.\n");
			write(fd2[WRITE_END], number, sizeof(int));
			
			printf("2nd (child) process finishes\n");
			// close the write end of the pipe which is an unused end of the pipe
			close(fd2[WRITE_END]);
		}
	}	
	return 0;
}
