#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int **increaseSize(int **arr, int n, int m)
{
  arr = realloc(arr, n);
  for(int i = 0; i < n; i++)
  {
    arr[i] = realloc(arr[i], m); 
  }
  return arr;
}

int main(int argc, char **argv)
{
  int noProcesses = 0, sizeP = 20, sizeT = 20;
  size_t len = 0;
  int** burstTimes = calloc(sizeP, sizeof(int*));
  for (int index = 0; index < sizeP; index++)
  {
    burstTimes[index] = calloc(sizeT, sizeof(int));
	}
  
	char *command = NULL;
  while(getline( &command, &len, stdin ) != -1) 
	{
    // remove newlines
    char *newline = strchr(command, '\n');
    if (newline) *newline = '\0';
    
    //split command into separate strings using space as delimiter
    int i = 0;
    char *token = strtok(command, " ");
    while (token != NULL)
    {
      int x = atoi(token);
      burstTimes[noProcesses][i] = x;
      token = strtok(NULL, " ");
      //printf("%s ", token);
      i++;
      if (i == sizeT)
      {
        sizeT *= 2;
        //burstTimes = increaseSize(burstTimes, sizeP, sizeT);
      }
		}
		noProcesses++;
    if (noProcesses == sizeP)
    {
      noProcesses *= 2;
      //burstTimes = increaseSize(burstTimes, sizeP, sizeT);
    }
    //printf("\n");
		//free(command);
	}
  
  for(int i = 0; i < noProcesses; i++)
  {
    for(int j = 0; j < sizeT; j++)
    {
      if (burstTimes[i][j] != -1 || burstTimes[i][j] != 0)
      {
        //printf("%d ", burstTimes[i][j]);
      }
    }
    //printf("\n");
  }
  
  //burstTimes
  int totalBurstTimes = 0;
  for(int i = 0; i < noProcesses; i++)
  {
    int j = 2;
    while(burstTimes[i][j] != -1)
    {
      totalBurstTimes += burstTimes[i][j];
      j++;
    }
  }
  
  //printf("Total Bursttime: %d\n", totalBurstTimes);
  
  //cpu waittime
  int cpuWaitTime = 0;
  for(int i = 1; i < noProcesses; i++)
  {
    //printf("Current cputime: %d\n", cpuWaitTime);
    for(int k = 1; k <= i; k++)
    {
      cpuWaitTime += burstTimes[i-k][2];;
    }
    cpuWaitTime -= burstTimes[i][0];
  }
  
  //printf("Total CPUTime: %d\n", cpuWaitTime);
  
  //io waittime
  int ioWaitTime = 0;
  for(int i = 0; i < noProcesses-1; i++)
  {
    int j = 3;
    if (burstTimes[i][j] == -1) break;
    while(burstTimes[i][j+1] != -1)
    {
      int times = 0;
      for(int k = i+1; k < noProcesses; k++)
      {
        //printf("Current p: %d j: %d k: %d\n", i, j, k);
        if(burstTimes[k][j-1] != -1 )
        {
          //printf("time to add: %d\n", burstTimes[k][j-1]);
          times += burstTimes[k][j-1];
        }
      }
      //printf("Current p: %d j: %d\n", i, j);
      if (times != 0)
      {
        ioWaitTime += abs(times - burstTimes[i][j]);
        //printf("Waittime: %d\n", abs(times - burstTimes[i][j]));
      }
      j++;
    }
  }
  
  //printf("Total IOTime: %d\n", ioWaitTime);
  //printf("Total Waittime: %d\n", cpuWaitTime + ioWaitTime);
  //printf("Total Procsesses: %d\n", noProcesses);
  printf("%d\n", (cpuWaitTime +  ioWaitTime + totalBurstTimes)/noProcesses);

  return 0;
}