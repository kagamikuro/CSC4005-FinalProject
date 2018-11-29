
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>




////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////

char *textData;
int textLength;

char *patternData;
int patternLength;

clock_t c0, c1;
time_t t0, t1;

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




int hostMatch(long *comparisons,int controlNumber,int textNumber,int patternNumber)
{
	int i,j,k, lastI;
	int counter = 0;
	i=0;
	j=0;
	k=0;
	lastI = textLength-patternLength;
	int position = -1;
	#pragma omp parallel for num_threads(4) private(k,j) shared(i)  	
		for(i=0;i<=lastI;i++){
			k=i;			
			j=0;
			if(textData[k] != patternData[j])
				continue;
		
				
			while(textData[k] == patternData[j] && j<patternLength &&k<textLength ){			
					k++;
					j++;
			}
			if(j == patternLength){
				position = i;
				if(controlNumber == 1){
					printf ("%d %d %d\n",textNumber,patternNumber,i);
		                						  			
				}												
			}							
			
		}		
	return position;
}
void processData(int controlNumber,int textNumber,int patternNumber)
{
	unsigned int result;
        long comparisons;

	//printf ("Text length = %d\n", textLength);
	//printf ("Pattern length = %d\n", patternLength);

	result = hostMatch(&comparisons,controlNumber,textNumber,patternNumber);
	
	if(controlNumber == 0){	
		if (result == -1){
			printf ("%d %d %d\n",textNumber,patternNumber,-1);
			
		}
		else{
			printf ("%d %d %d\n",textNumber,patternNumber,-2);
		
		}

	}
	if(controlNumber == 1){
		if(result == -1){
			printf ("%d %d %d\n",textNumber,patternNumber,-1);          
		}
	}
}

int main(int argc, char **argv)
{
	int n=0;
	int lines = readControlData();
	for(n=0;n<=lines;n++){
		//printf("control: %d %d %d\n",control[n][0],control[n][1],control[n][2]);
		readData (control[n][1],control[n][2]);
		//printf("pattern: %s\n",patternData);
		//printf("text: %s\n",textData);
		processData(control[n][0],control[n][1],control[n][2]);
	}
/*

	for(threads=1;threads<=64;threads=threads*2){
		printf("using %d threads:\n",threads);	
		c0 = clock(); t0 = time(NULL);	
   		processData(threads);
		c1 = clock(); t1 = time(NULL);
        	printf("using %d threads: on test0 elapsed wall clock time = %ld\n", threads, (long) (t1 - t0));
        	printf("using %d threads: elapsed CPU time = %f\n\n", threads, (float) (c1 - c0)/CLOCKS_PER_SEC); 
	}

*/
}
