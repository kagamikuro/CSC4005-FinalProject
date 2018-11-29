
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <mpi.h>

#define DIETAG 2

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////

char *textData;
int textLength;

char *patternData;
int patternLength;


void master();
void slave();


int control[1000000][3];


void outOfMemory()
{
	fprintf (stderr, "Out of memory\n");
	exit (0);
}


int readControlData(){
        FILE *f;
        char fileName[1000];
	int line = 0;
#ifdef DOS
        sprintf (fileName, "inputs\\control.txt");
#else
        sprintf (fileName, "inputs/control.txt");
#endif
        f = fopen (fileName, "r");
        if (f == NULL)
                exit(-1);

	while(!feof(f)){
		fscanf(f,"%d %d %d",&control[line][0],&control[line][1],&control[line][2]);
		line ++;
	}
	return line -2; //return total lines

}


void readFromFile (FILE *f, char **data, int *length)
{
	int ch;
	int allocatedLength;
	char *result;
	int resultLength = 0;

	allocatedLength = 0;
	result = NULL;

	

	ch = fgetc (f);
	while (ch >= 0)
	{
		resultLength++;
		if (resultLength > allocatedLength)
		{
			allocatedLength += 10000;
			result = (char *) realloc (result, sizeof(char)*allocatedLength);
			if (result == NULL)
				outOfMemory();
		}
		result[resultLength-1] = ch;
		ch = fgetc(f);
	}
	*data = result;
	*length = resultLength;
}



int readData (int textNumber,int patternNumber)
{
	FILE *f;
	char fileName[1000];
#ifdef DOS
        sprintf (fileName, "inputs\\text%d.txt", textNumber);
#else
	sprintf (fileName, "inputs/text%d.txt", textNumber);
#endif
	f = fopen (fileName, "r");
	if (f == NULL)
		return 0;
	readFromFile (f, &textData, &textLength);
	fclose (f);
#ifdef DOS
        sprintf (fileName, "inputs\\pattern%d.txt", patternNumber);
#else
	sprintf (fileName, "inputs/pattern%d.txt", patternNumber);
#endif
	f = fopen (fileName, "r");
	if (f == NULL)
		return 0;
	readFromFile (f, &patternData, &patternLength);
	fclose (f);

	//printf ("Read %d %d\n", textNumber,patternNumber);
	
	return 1;
	
}




int hostMatch(long *comparisons,int type,int textNumber,int patternNumber)
{
	int i,j,k, lastI;
	int result = -1;
	i=0;
	j=0;
	k=0;
	lastI = textLength-patternLength;
        *comparisons=0;

	while (i<=lastI)
	{
                (*comparisons)++;
		if (textData[k] == patternData[j])
		{
			k++;
			j++;
		}
		else
		{
			i++;
			k=i;
			j=0;
		}
		if (j == patternLength){//means pattern has been found
			result = i;
			if(type == 1){
				printf ("%d %d %d\n",textNumber,patternNumber,i);
		     }
		}
	}

	return result;	
	
}

void processData(int type,int textNumber,int patternNumber)
{
	unsigned int result;
        long comparisons;
	
	result = hostMatch(&comparisons,type,textNumber,patternNumber);
				
	if(type == 0){	
		 if(result == -1){
			printf ("%d %d %d\n",textNumber,patternNumber,-1);
		}
		else{
			printf ("%d %d %d\n",textNumber,patternNumber,-2);
		}
	}
	if(type == 1){
		if(result == -1){
			printf ("%d %d %d\n",textNumber,patternNumber,-1);									}
	}										
}
int main(int argc, char **argv)
{
	int testNumber=1;
	int npes ,myrank;
	
	
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&npes);
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
	
	if(myrank == 0)
		master();
	else
		slave();

	MPI_Finalize();
}

void master(){
	int totalLines = readControlData();//read control data and get the total lines in control file
	int line = 0;
	int slaveNo = 1;//firstly set the slave number equals 1
	
	int sizes;
	MPI_Comm_size(MPI_COMM_WORLD,&sizes);//get the total process number	
	
	for(line=0;line<=totalLines;line++){
		//printf("%d %d %d\n",control[line][0],control[line][1],control[line][2]);
		int type = control[line][0];
	        int textNumber = control[line][1];
        	int patternNumber = control[line][2];
		

		MPI_Send(&type,1,MPI_INT,slaveNo,0,MPI_COMM_WORLD);//send type number to the slave firstly
		MPI_Send(&textNumber,1,MPI_INT,slaveNo,0,MPI_COMM_WORLD);//send text number to the slave then
		MPI_Send(&patternNumber,1,MPI_INT,slaveNo,0,MPI_COMM_WORLD);//send pattern number to the slave firstly		
				
		slaveNo++; //change to next slave
		if(slaveNo == sizes || slaveNo>sizes) // if the slaveNO exceed the maximum , set the slaveNo to one
			slaveNo = 1;

		
	}

	for(slaveNo = 1;slaveNo<sizes;slaveNo++)
               MPI_Send(0,0,MPI_INT,slaveNo,DIETAG,MPI_COMM_WORLD);//send DIETAG to each process to terminate their loop	
	
}

void slave(){


	int result;
	MPI_Status status;
	int myrank;
	int textLength;
	MPI_Comm_rank(MPI_COMM_WORLD,&myrank);

	int type;
        int textNumber;
        int patternNumber;
 
		
	while(1){
		MPI_Recv(&type,1,MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
		
		if(status.MPI_TAG == DIETAG)
                        return;//check the tag of the received message

		MPI_Recv(&textNumber,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);//receive pattern from master process

		MPI_Recv(&patternNumber,1,MPI_INT,0,0,MPI_COMM_WORLD,&status);
		
		//printf("slave %d received : %d %d %d\n",myrank,type,textNumber,patternNumber);	

		readData(textNumber,patternNumber);

		processData(type,textNumber,patternNumber);	

	}

}
