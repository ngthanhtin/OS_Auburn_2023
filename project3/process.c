// Project 3 - AUbatch - A batch scheduling system
// Thanh-Tin Nguyen
// 18/03/2023

// This file is used in execv
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  int burst_time;

  sscanf(argv[2], "%d", &burst_time);

  printf("A process is running ...\n");

  sleep(burst_time);
  return 0;
  
}

