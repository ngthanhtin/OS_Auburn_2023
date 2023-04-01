#include "cpmfsys.h"
#include "diskSimulator.h"
#include<string.h>

bool free_list[256];

/*makes a directory structure containing the file elements from the main memory.*/
/* Pointer *e was explained in the class taken from Dr. Qin's code. */

DirStructType *mkDirStruct(int index,uint8_t *e){

	DirStructType *d;
	/* using malloc to set up the block size was recommeded by Dr. Qin. */
	d = malloc(sizeof(DirStructType));
	int i;
	int j=0;
		(d -> status) = (e+index*EXTENT_SIZE)[0];
		j++;
		i=0;

		while(j<9){
			if ((e+index*EXTENT_SIZE)[j] != ' '){	
				(d -> name)[i] = (e+index*EXTENT_SIZE)[j];
			}
			else {
				(d -> name)[i] = '\0';
				break;
			}
			i++;
			j++;
		}
		i=0; 
		j=9;

		while(j<12){
			if ((e+index*EXTENT_SIZE)[j] != ' '){	
				d -> extension[i] = (e+index*EXTENT_SIZE)[j];
			}
			else {
				d -> extension[i] = '\0';
				break;
			}
			j++;
			i++;
		}
		d -> XL = (e+index*EXTENT_SIZE)[12];
		d -> BC = (e+index*EXTENT_SIZE)[13];
		d -> XH = (e+index*EXTENT_SIZE)[14];
		d -> RC = (e+index*EXTENT_SIZE)[15];

		j=16;
		i=0;
		while(j<32){
			d -> blocks[j] = (e+index*EXTENT_SIZE)[j];
			i++;
			j++;
		}
		return d;	
}

/*writes the directory structure into the main memory*/
void writeDirStruct(DirStructType *d,uint8_t index, uint8_t *e){	
	int i=0;
	int j;
	(e+index*EXTENT_SIZE)[0] = d -> status;
	i++;
	j=0;
	while(i<9){
		if (d -> name[j] != '\0'){	
			(e+index*EXTENT_SIZE)[i] = d -> name[j];
		}
		else {
			(e+index*EXTENT_SIZE)[i] = ' ';
		}
	    j++;
		i++;
	}
	j=0;
	while(i<12){
		if (d -> extension[j] != '\0'){	
			(e+index*EXTENT_SIZE)[i] = d -> extension[j];
		}
		else {
			(e+index*EXTENT_SIZE)[i] = ' ';
		}
	    j++;
		i++;
	}
	(e+index*EXTENT_SIZE)[12] = d -> XL;
	(e+index*EXTENT_SIZE)[13] = d -> BC;
	(e+index*EXTENT_SIZE)[14] = d -> XH;
	(e+index*EXTENT_SIZE)[15] = d -> RC;
	i=16;
	j=0;
	while(i<32){
		(e+index*EXTENT_SIZE)[i] = d -> blocks[j];
		i++;
		j++;
	}	
}

/*global free list array that contains a list of used and unused blocks*/
void makeFreeList(){

	uint8_t buffer[1024];
	int index,b_index;
	DirStructType *cpm_dir;	

	blockRead(buffer,(uint8_t)0);

	for(index;index<256;index++){
		free_list[index] = true;
	}

	for(index=0;index<32;index++){
		cpm_dir = mkDirStruct(index,buffer);
		for(b_index = 0;b_index<16;b_index++){
			if (cpm_dir -> blocks[b_index] == 0)
				free_list[(int)cpm_dir -> blocks[b_index]] = true;
			else
				free_list[(int)cpm_dir -> blocks[b_index]] = false;	
		}
	}
	free_list[0] = false;
	
}

//free dot
/*prints the freelist */

void printFreeList(){
	int index;
	char addr[16][3] = {"0","10","20","30","40","50","60","70","80",
	"90","a0","b0","c0","d0","e0","f0"};
	int j=0;
	int i=0;	
	printf("FREE BLOCK LIST: (* means in-use)\n");
	fprintf(stdout,"%s ",addr[0]);	
	i++;
	free_list[0]=false;
	while(j>=0 && j<256){
		if (j%16 == 0 && j!=0){
			fprintf(stdout,"\n");
			fprintf(stdout,"%s ",addr[i]);
			i++;
		}
		if (free_list[j]){
			fprintf(stdout,". ");
		}
		else{
			fprintf(stdout,"* ");
		}
		j++;
		
	}
	  fprintf(stdout,"\n");
}

/*checks if the file name is valid or not*/
bool checkLegalName(char *name){
	int i=0;
	int length=0;
	int ext_bit;
	length=strlen(name);
	if (name[0]!= ' ' || name[0] != '.'|| name[0] != '\0'){		
		while (name[i]!= '.' && i<8 && i<length){
			if ((name[i]>=65 && name[i]<=90) || (name[i] >=97 && name[i] <= 122) || (name[i] >=48 && name[i] <= 57) && i<8){
				i++;			
			}
			else {
				
				return false;
			}
		}
		if (name[i]=='.' && i<=8)
		{
			ext_bit = 0;
			i++;
			while(ext_bit<3 && i<12 && i<length)
			{				
				if ((name[i]>=65 && name[i]<=90) || (name[i] >=97 && name[i] <= 122) || (name[i] >=48 && name[i] <= 57) && (ext_bit<3)){
					
					i++;
					ext_bit++;				    
				}
				else {
					
					return false;
				}
			}
			
			return true;
		}
		else {
			
			return false;
		}
	
	return true;
	}
	else{
		
		return false;
	}
}

/*returns the index value of the file for which the data is to be accessed.*/
int findExtentWithName(char *name, uint8_t *block0){
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
	if (result == true){
		while(i>=0 && i<length){
			if (name[i]!='.'){
				sp_name[j]=name[i];
				j++;
				i++;			
			}
			else if (name[i] == '.'){
				i++;
				sp_name[j]='\0';
				j=0;
				while(i>0 && i<length){
					if (name[i]!=' '){
						sp_ext[j]=name[i];
						j++;
						i++;
					}
					else if (name[i]==' '){
						sp_ext[j]='\0';
						break;
					}
					
				}
				sp_ext[j]='\0';
			}
		}
		for(index=0;index<32;index++){
			cpm_dir = mkDirStruct(index,block0);
				if (((strcmp(sp_name,cpm_dir->name))==0) && ((strcmp(sp_ext,cpm_dir->extension))==0)){
					if(cpm_dir -> status != 0xe5){
						return index;
					}
					else
						return -1;
				}			
			}
	}	
		else 
			return false;
	}

/*gives the directory structure with the file length for each file*/

void cpmDir(){

	int index, b_index;
	int b_num;
	int filelen = 0;
	uint8_t buffer[1024];
	DirStructType *cpm_dir;	

	blockRead(buffer,(uint8_t)0);

	printf("DIRECTORY LISTING\n");

	for(index=0; index<32; index++){
		cpm_dir = mkDirStruct(index,buffer);
		if(cpm_dir -> status != 0xe5)
		{
			b_num = 0;
			for(b_index = 0; b_index<16; b_index++){

				if (cpm_dir -> blocks[b_index]!=0)
					b_num++;
			}
			filelen = (b_num -1)*1024+(cpm_dir -> RC)*128+(cpm_dir -> BC);

			fprintf(stdout,"%s.%s %d\n",cpm_dir->name,cpm_dir->extension,filelen);
		}
	}
}

/*	delete a filename and its allocated blocks with the filename passed as a parameter*/
int  cpmDelete(char * name){

	uint8_t buffer[1024];
	DirStructType *cpm_dir;
	int result=0;
	int index;

	blockRead(buffer,(uint8_t)0);

	result= findExtentWithName(name,buffer);

	if (result!=-1){		
	    cpm_dir = mkDirStruct(result,buffer);
		cpm_dir -> status = 0xe5;
		index=0;
		while(index<16){		
			free_list[(int)cpm_dir -> blocks[index]] = true;	
			cpm_dir -> blocks[index] = 0;			
			index ++;
		}
	writeDirStruct(cpm_dir,result,buffer);
	blockWrite(buffer,(uint8_t)0); 
	return 0;
	
	}
	else{
		return -1;
	}
}

/*rename a filename with the old filename and new filename passed as parameters*/
int cpmRename(char *old_Name, char * new_Name){
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
	if (check == false){
		return -2;
	}
	result= findExtentWithName(old_Name,buffer);
	if (result!=-1){
		cpm_dir = mkDirStruct(result,buffer);
		while(i>=0 && i<length){
			if (new_Name[i]!='.'){
				cpm_dir -> name[j] = new_Name[i];				
				i++;
				j++;
			}
			else if (new_Name[i] == '.'){
				i++;
				cpm_dir -> name[j]='\0';
				j=0;
				while(i>0 && i<length){
					if (new_Name[i]!=' '){
						cpm_dir -> extension[j] = new_Name[i];
						j++;
						i++;
					}
					else if (new_Name[i]==' '){
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
