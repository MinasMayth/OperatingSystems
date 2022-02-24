#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

void executeCommand(char* command[][3], int numberOfCommands, char** operators)
{
	int child,status, skip = 0, indexOfCommand = 0;
	
	char **nextCommand;
	
	while(indexOfCommand != numberOfCommands) {
		 
		if(skip)
		{
			indexOfCommand++;
			skip = 0;
		}
		
		nextCommand = command[indexOfCommand];
		if (!strcmp(nextCommand[0],"exit")) exit(0);
		
		int savedStdOut = dup(1), savedStdIn = dup(0); //save old file descriptors before changing
		
		if (strchr(nextCommand[1], '<') != NULL || strchr(nextCommand[1], '>')  != NULL)
		{
			char test[50];
			strcpy(test, nextCommand[1]);
			char *token = strtok(test, " ");
			char *files[4];
			
			int i = 0;
			while(token != NULL)
			{
				files[i] = token;
				token = strtok(NULL, " ");
				i++;
			}
			
			free(token);
			
			if (strcmp(files[0],"<") == 0)
			{
				int fd0 = open(files[1], O_RDONLY);
				dup2(fd0, STDIN_FILENO);
				close(fd0);
			}
			
			if (strcmp(files[0],">") == 0)
			{
				int fd1 = creat(files[1] , 0644) ;
				dup2(fd1, STDOUT_FILENO);
				close(fd1);
			}
			
			if (strcmp(files[2],">") == 0)
			{
				int fd1 = creat(files[3] , 0644) ;
				dup2(fd1, STDOUT_FILENO);
				close(fd1);
			}
		}

		if((child=fork())==0) {
			// execute the command entered by the user
			execvp(nextCommand[0], nextCommand);
			printf("Error: command not found!");
			exit(-1);
		} 
		else 
		{
			waitpid(-1,&status,0);
			
			//restore old file descriptors
			dup2(savedStdOut, 1);
			close(savedStdOut);
			dup2(savedStdIn, 0);
			close(savedStdIn);
			
			if(WEXITSTATUS(status) != 0)
			{
				if(numberOfCommands > 1) if(!strcmp(operators[indexOfCommand], "&&")) skip = 1; //command failed and skip next
			}
			else
			{
				if(numberOfCommands > 1) if (!strcmp(operators[indexOfCommand], "||")) skip = 1; //command succes but skip anyway
			}
		}
		indexOfCommand++;
	}
}

int main(int argc, char **argv)
{
  //char* command[][3] = {{"echo", "a", NULL}, {"echo", "b", NULL}, {"echo", "c", NULL}, {"echo", "d", NULL}};;
  //char* operators[] = {"||", "&&", ";"};
  
  //char* command[][3] = {{"./b.out", "< in", NULL}, {"echo", "a", NULL}, {"echo", "b", NULL} };
  //char* operators[] = {"&&",";"};
  
  //char* command[][3] = {{"echo", "some string <> with 'special' characters", NULL}, {"echo", "b", NULL}};
  //char* operators[] = {"&&"};
  
  char* command[][3] = {{"exit","", NULL}};
  char* operators[] = {};
  
  //char* command[][3] = {{"echo", "x", NULL}};
  //char* operators[] = {};
  
  executeCommand(command, 1, operators);
  exit(0);
}
