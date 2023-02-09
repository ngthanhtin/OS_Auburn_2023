/*
 * COMP 7500: Project 2: pWordCount: A Pipe-based WordCount Tool 
 * Thanh Tin Nguyen - 904285164 
 * ttn0011@auburn.edu
 * Date: 2/9/2023
 * Auburn University
*/

/*
 * This is the header file of utils.c, contains the initialization of functions written in utils.c
*/

int count_words(char *word); // used to count the number of words in a file
char* check_filename_extension(char *filename); // check filename's extension
void error_checking(int error_type, char *additional_msg); // print the error, and any additional message
