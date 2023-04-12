#include "cpmfsys.h"
#include "diskSimulator.h"
#include <string.h>
#include <stdint.h> 
#include <stdlib.h> 
#include  <stdbool.h> 
#include <stdio.h> 

// define constant variables
#define EXTENT_SIZE 32
#define BLOCKS_PER_EXTENT 16 
// define global variables
bool free_list[256];

/*makes a directory structure containing the file elements from the main memory.*/
/* Pointer *e was explained in the class taken from Dr. Qin's code. */

DirStructType *mkDirStruct(int index,uint8_t *e) 
{
	/* using malloc to set up the block size was recommeded by Dr. Qin. */
	DirStructType *d = malloc(sizeof(d));
	//status
	uint8_t *loc = (e+index*EXTENT_SIZE);
	d -> status = loc[0];

	int i = 0;
	int j = 1;//start byte index of filename
	//name
	while (j <= 9)
	{
		if (loc[j] != ' ')
			d -> name[i] = loc[j];
		i++;
		j++;
	}
	//add terminator to filename
	d -> name[8] = '\0';
	
	//extension
	i = 0;
	j = 9; // start byte index of extension
	while(j <= 12)
	{
		if (loc[j] != ' ')
			d -> extension[i] = loc[j];
		i++;
		j++;
	}
	//add teminator to extension
	d -> extension[3] = '\0';
	
	//4 bytes
	d -> XL = loc[12];
	d -> BC = loc[13];
	d -> XH = loc[14]; 
	d -> RC = loc[15];

	//block
	i = 0;
	j = 16; // start byte index of blocks
	for(i = 0; i < BLOCKS_PER_EXTENT; i++) 
	{
		d -> blocks[i] = loc[j];
		j++;
	}
	
	return d;
}	

/*gives the directory structure with the file length for each file*/

void cpmDir()
{
	int i, block_i;
	int block_num = 0;
	int file_len = 0;
	uint8_t *block0 = malloc(1024);
	blockRead(block0, 0);
	
	printf("DIRECTORY LISTING\n");

	for(i=0; i<32; i++)
	{
		DirStructType *d = mkDirStruct(i, block0);
		if(d -> status != 0xe5)
		{
			block_num = 0;
			for(block_i = 0; block_i < BLOCKS_PER_EXTENT; block_i++)
			{
				if (d -> blocks[block_i]!=0)
					block_num++;
			}
			file_len = (block_num -1)*BLOCK_SIZE + d -> RC *128 + d -> BC;

			fprintf(stdout,"%s.%s %d\n",d->name, d->extension,file_len);
		}
	}
}

/*writes the directory structure into the main memory*/
void writeDirStruct(DirStructType *d,uint8_t index, uint8_t *e)
{	
	int i=0;
	int j;
	uint8_t *loc = (e+index*EXTENT_SIZE);
	loc[0] = d -> status;
	i++;
	j=0;
	while(i < 9)
	{
		if (d -> name[j] != '\0')
		{	
			loc[i] = d -> name[j];
		}
		else 
		{
			loc[i] = ' ';
		}
	    j++;
		i++;
	}
	j=0;
	while(i<12)
	{
		if (d -> extension[j] != '\0')
		{	
			loc[i] = d -> extension[j];
		}
		else 
		{
			loc[i] = ' ';
		}
	    j++;
		i++;
	}
	loc[12] = d -> XL;
	loc[13] = d -> BC;
	loc[14] = d -> XH;
	loc[15] = d -> RC;
	i=16;
	j=0;
	while(i<32)
	{
		loc[i] = d -> blocks[j];
		i++;
		j++;
	}	
}

/*global free list array that contains a list of used and unused blocks*/
// populate the FreeList global data structure. freeList[i] == true means 
// that block i of the disk is free. block zero is never free, since it holds
// the directory. freeList[i] == false means the block is in use. 
void makeFreeList()
{

	uint8_t *block0 = malloc(1024);
	int i, block_i;
	
	free_list[0] = false;

	for(i = 1;i < 256; i++)
	{
		free_list[i] = true;
	}

	// extent
	blockRead(block0, 0);

	for(i = 0;i < 32; i++)
	{
		DirStructType *extent = mkDirStruct(i, block0);
		if (extent -> status != 0xe5)
		{
			for(block_i = 0;block_i < BLOCKS_PER_EXTENT; block_i++)
			{
					if (extent -> blocks[block_i] != 0)
						free_list[(int)extent -> blocks[block_i]] = false;	
			}
		}
	}
}

/*prints the freelist */
void printFreeList()
{
	int i=0;	
	int j=0;
	printf("FREE BLOCK LIST: (* means in-use)\n");
	fprintf(stdout,"%x0: ",0);	
	i++;
	free_list[0] = false;
	while(j>=0 && j<256)
	{
		if (j%16 == 0 && j!=0)
		{
			fprintf(stdout,"\n");
			fprintf(stdout,"%x0: ",i);
			i++;
		}
		if (free_list[j] == true)
		{
			fprintf(stdout,". ");
		}
		else
		{
			fprintf(stdout,"* ");
		}
		j++;
		
	}
	fprintf(stdout,"\n");
}

/*
Checks if the file name is valid or not, a file name is valid if:
no blank, no special characters, no punctuation, no controls, etc
*/
bool checkLegalName(char *name)
{
	int i=0;
	int length=0;
	int ext_bit;
	length=strlen(name);
	if (name[0]!= ' ' || name[0] != '.'|| name[0] != '\0')
	{		
		while (name[i]!= '.' && i<8 && i<length)
		{
			if ((name[i]>=65 && name[i]<=90) || (name[i] >=97 && name[i] <= 122) || (name[i] >=48 && name[i] <= 57) && i<8)
			{
				i++;			
			}
			else 
			{
				
				return false;
			}
		}
		if (name[i]=='.' && i<=8)
		{
			ext_bit = 0;
			i++;
			while(ext_bit<3 && i<12 && i<length)
			{				
				if ((name[i]>=65 && name[i]<=90) || (name[i] >=97 && name[i] <= 122) || (name[i] >=48 && name[i] <= 57) && (ext_bit<3))
				{
					
					i++;
					ext_bit++;				    
				}
				else 
				{
					return false;
				}
			}
			
			return true;
		}
		else 
		{
			
			return false;
		}
	
		return true;
	}
	else
	{
		return false;
	}
}

/*returns the index value of the file for which the data is to be accessed.*/
int findExtentWithName(char *name, uint8_t *block0)
{
	bool result;
	int index;
	int i=0;
	int j=0;
	char sp_name[9];
	char sp_ext[4];
	DirStructType *cpm_dir;
	char sp[18];
	int length=0;
	length=strlen(name);
	result = checkLegalName(name);
	// Add '\0' to the end of filename and file extension name
	if (result == true)
	{
		while(i>=0 && i<length)
		{
			if (name[i]!='.')
			{
				sp_name[j]=name[i];
				j++;
				i++;			
			}
			else if (name[i] == '.')
			{
				i++;
				sp_name[j]='\0';
				j=0;
				while(i>0 && i<length)
				{
					if (name[i]!=' ')
					{
						sp_ext[j]=name[i];
						j++;
						i++;
					}
					else if (name[i]==' ')
					{
						sp_ext[j]='\0';
						break;
					}
					
				}
				sp_ext[j]='\0';
			}
		}
		for(index=0;index<32;index++)
		{
			cpm_dir = mkDirStruct(index,block0);
			if (((strcmp(sp_name,cpm_dir->name))==0) && ((strcmp(sp_ext,cpm_dir->extension))==0))
			{
				if(cpm_dir -> status != 0xe5)
				{
					return index;
				}
			}			
		}
		return -1;
	}	
	else 
	{
		printf("This file name is not valid.\n");
		return false;
	}
		
}



/*	delete a filename and its allocated blocks with the filename passed as a parameter*/
int  cpmDelete(char * name)
{

	uint8_t buffer[1024];
	DirStructType *cpm_dir;
	int result=0;
	int index;

	blockRead(buffer,(uint8_t)0);

	result= findExtentWithName(name,buffer);

	if (result!=-1)
	{		
	    cpm_dir = mkDirStruct(result,buffer);
		cpm_dir -> status = 0xe5;
		index=0;
		while(index<16)
		{		
			free_list[(int)cpm_dir -> blocks[index]] = true;	
			cpm_dir -> blocks[index] = 0;			
			index ++;
		}
		writeDirStruct(cpm_dir,result,buffer);
		blockWrite(buffer,(uint8_t)0); 
		return 0;
	
	}
	else
	{
		return -1;
	}
}

/*rename a filename with the old filename and new filename passed as parameters*/
int cpmRename(char *old_Name, char * new_Name)
{
	uint8_t buffer[1024];

	int i=0;
	int length=0;
	int j=0;
	int result=0;
	bool check;
	length = strlen(new_Name);
	DirStructType *cpm_dir;
	blockRead(buffer,(uint8_t)0);
	check = checkLegalName(new_Name);
	if (check == false)
	{
		return -2;
	}
	result= findExtentWithName(old_Name,buffer);
	if (result!=-1)
	{
		cpm_dir = mkDirStruct(result,buffer);
		while(i>=0 && i<length)
		{
			if (new_Name[i]!='.')
			{
				cpm_dir -> name[j] = new_Name[i];				
				i++;
				j++;
			}
			else if (new_Name[i] == '.')
			{
				i++;
				cpm_dir -> name[j]='\0';
				j=0;
				while(i>0 && i<length)
				{
					if (new_Name[i]!=' ')
					{
						cpm_dir -> extension[j] = new_Name[i];
						j++;
						i++;
					}
					else if (new_Name[i]==' ')
					{
						cpm_dir -> extension[j]='\0';
						break;
					}
					
				}
				cpm_dir -> extension[j]='\0';
			}
		}
		writeDirStruct(cpm_dir,result,buffer);	
		blockWrite(buffer,(uint8_t)0); 
		return 0;
	}
	else 
		return -1;
}
