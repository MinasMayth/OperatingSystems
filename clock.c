#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void addToFrame(int x, int **frames, int n, int *firstIn)
{
  int added = 0;
  for(int i = 0; i < n; i++)
  {
    if(frames[i][0] == -1)
    {
      frames[i][0] = x;
      frames[i][0] = 0;
      added = 1;
      (*firstIn) = i+1;
      break;
    }
  }
  
  if(added == 0) 
  {
    frames[(*firstIn)][0] = x;
    frames[(*firstIn)][1] = 0;
    (*firstIn)++;
  }
}

int inFrame(int x, int **frames, int n, int *firstIn)
{
  for(int i = 0; i < n; i++)
  {
    if(frames[i][0] == x) 
    {
      frames[i][1] = 1;
      (*firstIn) = i;
      return 1;
    }
  }
  return 0;
}

int *getString(int *index)
{
  int size = 10;
  size_t len = 0;
  int* referenceString = calloc(size, sizeof(int));
  char *command = NULL;
  while(getline( &command, &len, stdin ) != -1) 
	{
    // remove newlines
    char *newline = strchr(command, '\n');
    if (newline) *newline = '\0';
    
    //split command into separate strings using space as delimiter
    (*index) = 0;
    char *token = strtok(command, " ");
    while (token != NULL)
    {
      int x = atoi(token);
      referenceString[(*index)] = x;
      token = strtok(NULL, " ");
      (*index)++;
      if ((*index) == size)
      {
        size *= 2;
        referenceString = realloc(referenceString, size * sizeof(int));
      }
    }
	}
  free(command);
  return referenceString;
}

int main(int argc, char **argv)
{
  int pages = 0, index = 0, firstIn = 0, pageFaults = 0;
  int* referenceString;
  
  scanf("%d", &pages);
  
  int **pageFrames = malloc(pages * sizeof(int*));
  for (int i = 0; i < pages; i++)
  {
    pageFrames[i] = malloc(2 * sizeof(int));
  }
  for(int i = 0; i < pages; i++)
  {
    pageFrames[i][0] = -1;
    pageFrames[i][1] = 0;
  }
  
  referenceString = getString(&index);
  
  //Loop over refrencestring
  for(int i = 0; i < index; i++)
  {
    if(!inFrame(referenceString[i], pageFrames, pages, &firstIn))
    {
      addToFrame(referenceString[i], pageFrames, pages, &firstIn);
      pageFaults++;
      if(firstIn == pages) firstIn = 0;
    }
  }
  
  printf("%d\n", pageFaults);
  free(referenceString);
  for(int i = 0;i < 2;i++)
  {
    free(pageFrames[i]);
  }
  free(pageFrames);
  
}