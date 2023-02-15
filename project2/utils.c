/* COMP 7500: Project 2: pWordCount: A Pipe-based WordCount Tool 
 * Thanh Tin Nguyen - 904285164 
 * ttn0011@auburn.edu
 * Date: Date: 2/10/2023
 * Auburn University
 */
#include<stdio.h>
#include<stdlib.h>
#include <stdbool.h>
#include<string.h>
#include<unistd.h>


#include "utils.h"
/*
 This file contains all utilities functions used in pwordcount program
*/

int count_words(char *word)
{
        int num_words = 0; // this var is to store the number of words
        int len = strlen(word); // get the length of string word
        
        int pos; // this var shows the position of the pointer in the string
        bool pointer_still_in_a_word = false; // this var is to tell if the current position of the pointer 
                // is still pointing to a character except 
                // '\0' (null), ' ' (space), '\t' (tab), '\n' (new line), '\r' (return)

	for(pos = 0; pos < len; pos++) // the pointer starts traversing the string from the beginning
        {
                char cur_char = word[pos]; // this var is to store the current character
                if(cur_char == '\0' || cur_char == ' ' || cur_char == '\t' 
                 || cur_char == '\n' || cur_char == '\r')
                {
			if (pointer_still_in_a_word) 
			{ 
				pointer_still_in_a_word = false; 
				num_words++; 
			}
		}
		else
                {
                        pointer_still_in_a_word = true;
                }
        }       

        return num_words;
}

char* check_filename_extension(char *filename)
{
        //This function will return empty string denoted that if there is no filename extension
        //It will return the file name extension if there is a filename extension

        //Note that: If "." is the first character in filename. Ex: ".abcdef", 
        //then, this file is a hidden file, and the filename extension will be "abcdef"

        char *dot = strrchr(filename, '.'); // search for the last occurrence of the character "."
        int len = strlen(filename); // get the length of filename
        
        if(!dot)// can not find the "." in filename. Ex: "abcdef"
        {
                printf("Can not find the \".\" in filename.\n");
                return "";
        }
        else if('.' == filename[len-1]) // "." is the last character in filename. Ex: "abcdef."
        {
                printf("\".\" is the last character in filename\n");
                return "";
        }

        return dot + 1; // get the string (filename extension)
}

void error_checking(int error_type, char *additional_msg)
{
        switch(error_type)
        {
                case 1: // Forget filename
                        printf("Please enter a file name.\nUsage: ./pwordcount <file_name>\n");
                        break;
                case 2: // too many input arguments
                        printf("Too many input arguments, please check again.\nUsage: ./pwordcount <file_name>\n");
                        break;
                case 3: // wrong file extension name (in this project, only using text file (*.txt))
                        printf("Wrong input file type, please use a text file.\nUsage: <file_name>\n");
                        break;
                case 4: // can't load file
                        // The additional message might be:
                        // 1 Operation not permitted
                        // 2 No such file or directory
                        // 3 No such process
                        // 4 Interrupted system call
                        printf("Error opening file: %s\n", additional_msg);
                        printf("Load file failed.\n");
                        break;
        }
}
