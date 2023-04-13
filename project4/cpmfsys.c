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
bool free_list[256]; // an array that contains a list of used and unused blocks

// makes a directory structure containing the file elements from the main memory
DirStructType *mkDirStruct(int index,uint8_t *e) 
{
	// using malloc to set up the block size was recommeded by Dr. Qin
	DirStructType *d = malloc(sizeof(d));
	//status
	uint8_t *loc = (e+index*EXTENT_SIZE);
	d -> status = loc[0];

	int i = 0;
	int j = 1;//start byte index of filename bytes
	//name
	while (j <= 9)
	{
		if (loc[j] != ' ')
			d -> name[i] = loc[j];
		i++;
		j++;
	}
	//add terminator to filename bytes
	d -> name[8] = '\0';
	
	//extension
	i = 0;
	j = 9; // start byte index of extension bytes
	while(j <= 12)
	{
		if (loc[j] != ' ')
			d -> extension[i] = loc[j];
		i++;
		j++;
	}
	//add teminator to extension bytes
	d -> extension[3] = '\0';
	
	//4 bytes
	d -> XL = loc[12];
	d -> BC = loc[13];
	d -> XH = loc[14]; 
	d -> RC = loc[15];

	//block
	i = 0;
	j = 16; // start byte index of block bytes
	for(i = 0; i < BLOCKS_PER_EXTENT; i++) 
	{
		d -> blocks[i] = loc[j];
		j++;
	}
	
	return d;
}	

// gives the directory structure with the file length for each file
void cpmDir()
{
	int i, block_i;
	int block_num = 0;
	int file_len = 0;
	uint8_t *block0 = malloc(BLOCK_SIZE);
	blockRead(block0, 0);
	
	printf("DIRECTORY LISTING\n");

	for(i=0; i < EXTENT_SIZE; i++)
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

// writes the directory structure into the main memory 
void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e)
{	
	int i=0;
	int j;
	uint8_t *loc = (e+index*EXTENT_SIZE);
	loc[0] = d -> status;
	
	j=0;
	i = 1; //start byte index of filename bytes
	//name
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
	i = 9; // start byte index of extension bytes
	//extension
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
	// 4 bytes
	loc[12] = d -> XL;
	loc[13] = d -> BC;
	loc[14] = d -> XH;
	loc[15] = d -> RC;

	i=16;// start byte index of block bytes
	j=0;
	while(i < EXTENT_SIZE)
	{
		loc[i] = d -> blocks[j];
		i++;
		j++;
	}	
}


// make free list that contains a list of used and unused blocks
// Note that; block0 is never free, since it holds the directory. 
// free_list[i] == true -> ith block is free 
// free_list[i] == false -> ith block is in use. 
void makeFreeList()
{

	uint8_t *block0 = malloc(BLOCK_SIZE);
	int i, block_i;
	
	free_list[0] = false;

	for(i = 1;i < NUM_BLOCKS; i++)
	{
		free_list[i] = true;
	}

	// extent
	blockRead(block0, 0);

	for(i = 0;i < EXTENT_SIZE; i++)
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

// prints the freelist
void printFreeList()
{
	int i=0;	
	int j=0;
	printf("FREE BLOCK LIST: (* means in-use)\n");
	fprintf(stdout,"%x0: ",0);	
	i++;
	free_list[0] = false;
	while(j >= 0 && j < NUM_BLOCKS)
	{
		if (j % BLOCKS_PER_EXTENT == 0 && j!=0)
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


// checks if the file name is valid or not, a file name is valid if:
// no blank, no special characters, no punctuation, no controls, etc

bool checkLegalName(char *name)
{
	int i=0; // filename index
	int length=0; // length of name
	int extension_i; // extension index
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
		if (name[i]=='.' && i <= 8)
		{
			extension_i = 0;
			i++;
			while(extension_i<3 && i<12 && i<length)
			{				
				if ((name[i]>=65 && name[i]<=90) || (name[i] >=97 && name[i] <= 122) || (name[i] >=48 && name[i] <= 57) && (extension_i<3))
				{
					
					i++;
					extension_i++;				    
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
	return false;
	
}

// returns the index value of the file for which the data is to be accessed
int findExtentWithName(char *name, uint8_t *block0)
{
	bool result;
	int index;
	int i=0;
	int j=0;
	char sp_name[9];
	char sp_ext[4];
	DirStructType *extent;
	char sp[18];
	int length=0;
	length=strlen(name);
	result = checkLegalName(name);
	// Add '\0' (terminator) to the end of filename and file extension name
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
		for(index = 0;index < EXTENT_SIZE; index++)
		{
			extent = mkDirStruct(index, block0);
			if (((strcmp(sp_name,extent->name))==0) && ((strcmp(sp_ext,extent->extension))==0))
			{
				if(extent -> status != 0xe5)
				{
					return index;
				}
			}			
		}
		return -1;
	}	
	else 
	{
		printf("Can not find this file name \" %s \" .\n", name);
		return -1;
	}
		
}

// rename an old filename by a new filename
int cpmRename(char *old_name, char *new_name)
{
	uint8_t *block0 = malloc(BLOCK_SIZE);

	int i=0;
	int j=0;

	int length=0;
	bool check; // used to check if the old_name or new_name is valid
	length = strlen(new_name);

	check = checkLegalName(old_name);
	if (check == false)
	{
		return -2;
	}
	check = checkLegalName(new_name);
	if (check == false)
	{
		return -2;
	}

	DirStructType *extent;
	blockRead(block0, 0);
	
	int result=0;
	result = findExtentWithName(old_name, block0);
	if (result!=-1)
	{
		extent = mkDirStruct(result, block0);
		while(i >= 0 && i < length)
		{
			if (new_name[i] != '.')
			{
				extent -> name[j] = new_name[i];				
				i++;
				j++;
			}
			else if (new_name[i] == '.')
			{
				i++;
				extent -> name[j] = '\0';
				j=0;
				while(i > 0 && i < length)
				{
					if (new_name[i] != ' ')
					{
						extent -> extension[j] = new_name[i];
						j++;
						i++;
					}
					else if (new_name[i] == ' ')
					{
						extent -> extension[j] = '\0';
						break;
					}
					
				}
				extent -> extension[j] = '\0';
			}
		}
		writeDirStruct(extent,result, block0);	
		blockWrite(block0, 0); 
		return 0;
	}
	return -1;
}

// delete a filename and its allocated blocks
int  cpmDelete(char * name)
{
	uint8_t *block0 = malloc(BLOCK_SIZE);
	DirStructType *extent;
	blockRead(block0, 0);

	int result = 0;
	result = findExtentWithName(name, block0);

	if (result!=-1)
	{		
	    extent = mkDirStruct(result, block0);
		extent -> status = 0xe5;
		int i = 0;
		while(i < BLOCKS_PER_EXTENT)
		{		
			free_list[(int)extent -> blocks[i]] = true;	
			extent -> blocks[i] = 0;			
			i++;
		}
		writeDirStruct(extent, result, block0);
		blockWrite(block0, 0); 
		return 0;
	
	}
	return -1;
}
