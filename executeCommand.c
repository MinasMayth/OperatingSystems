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
		  
		//printf("skip?: %d\n", skip);
		if(skip)
		{
			//take next command out of list
			indexOfCommand++;
			skip = 0;
		}
		
		nextCommand = command[indexOfCommand];
		
		int savedStdOut = dup(1), savedStdIn = dup(0);
		
		if (strchr(nextCommand[1], '<') != NULL)
		{
			printf("Found inputfile\n");
			int fd0 = open("in", O_RDONLY);
			dup2(fd0, STDIN_FILENO);
			close(fd0);
		}
		
		if (strchr(nextCommand[1], '>') != NULL)
		{
			printf("Found outputfile\n");
			int fd1 = creat("out" , 0644) ;
			dup2(fd1, STDOUT_FILENO);
			close(fd1);
		}
		
		//printf("nextCommand: %s\n", nextCommand[0]);

		if((child=fork())==0) {
			// execute the command entered by the user
			execvp(nextCommand[0], nextCommand);
			printf("Command %s not found!\n", nextCommand[0]);
			exit(-1);
		} 
		else 
			{
			waitpid(-1,&status,0);
			
			dup2(savedStdOut, 1);
			close(savedStdOut);
			
			dup2(savedStdIn, 0);
			close(savedStdIn);
			
			
			if(WEXITSTATUS(status) != 0)
			{
				printf("child exited with = %d\n",WEXITSTATUS(status));
				if(operators[indexOfCommand] == "&&") skip = 1; //command failed and skip next
			}
		}
		if (operators[indexOfCommand] == "||") skip = 1;
		else if (operators[indexOfCommand] == ";") skip = 0;
		indexOfCommand++;
	}
}

int main(int argc, char **argv)
{
  //char* command[][3] = {{"echo", "a", NULL}, {"echo", "a", NULL}, {"echo", "c", NULL}, {"echo", "d", NULL}};;
  //char* operators[] = {"||", "&&", ";"};
  
  char* command[][3] = {{"./b.out", "< in > out", NULL}, {"echo", "a", NULL}, {"echo", "b", NULL} };
  char* operators[] = {"||",";"};
  
  executeCommand(command, 3, operators);
}
