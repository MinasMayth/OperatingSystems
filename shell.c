#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

void trimTrailingWhiteSpace(char *str)
{
	int j = 0;
	for(int i = 0; str[i] != '\0'; i++)
	{
		if (str[i] != ' '&& str[i] != '\t') j = i;
	}
	str[j+1] = '\0';
}

int executeCommand(char* cmd, char* arg, int size)
{
	int child,status;
	int savedStdOut = dup(1), savedStdIn = dup(0); //save old file descriptors before changing
	
	trimTrailingWhiteSpace(cmd);
	trimTrailingWhiteSpace(arg);
	
    char *newline = strchr(arg, '\n');
	if (newline) *newline = '\0';
	
	newline = strchr(cmd, '\n');
	if (newline) *newline = '\0';
	
	if(cmd[0] == '\0') return 1;
	if(!strcmp(cmd, "exit")) return 2;
	
	if (strchr(arg, '<') != NULL || strchr(arg, '>')  != NULL)
	{
		char test[50];
		strcpy(test, arg);
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
	int n = 10;
	if (size > n) n = size;
	char **command = malloc(3 * sizeof(char*));
	for (int i = 0; i < 2; i++)
	{
		command[i] = malloc(n * sizeof(char));
	}
	
	strcpy(command[0], cmd);
	strcpy(command[1], arg);
	command[2] = NULL;
    
	if((child=fork())==0) {
		// execute the command entered by the user
		execvp(command[0], command);
		printf("Error: command not found!\n");
		
		//restore old file descriptors
		dup2(savedStdOut, 1);
		close(savedStdOut);
		dup2(savedStdIn, 0);
		close(savedStdIn);
		
		exit(-1);
	} 
	else 
	{
		waitpid(-1,&status,0);
		
		free(command[0]);
		free(command[1]);
		free(command);
		
		if(WEXITSTATUS(status) != 0)
		{
			return 0;
		}
		else
		{
			//restore old file descriptors
			dup2(savedStdOut, 1);
			close(savedStdOut);
			dup2(savedStdIn, 0);
			close(savedStdIn);
			return 1;
		}
	}
}

char *readCommand()
{
	char *line = NULL;
	size_t len = 0;
	
	if(getline(&line, &len, stdin) == -1)
	{
		if(feof(stdin))
		{
			exit(EXIT_SUCCESS);
		}
		else
		{
			perror("readline");
			exit(EXIT_FAILURE);
		}
	}
	return line;
}

void fineParse(char *line)
{
	int i = 0, n = 50, skip = 0, status = 0, size = 0;
	
	char *cmd = malloc(n * sizeof(char));
	char *arg = malloc(n * sizeof(char)); 
	cmd[0] = '\0'; 
	arg[0] = '\0';
	
	while (line[i] != '\0')
	{	
		if (line[i] == ' ') 
		{
			i++; 
			continue;
		}
		
		while(line[i] != ' ' && line[i] != '\0' && line[i] != '\n')
		{
			strncat(cmd, &line[i], 1);
			i++; 
		}
		
		if (line[i] == '\0') 
		{
			if (skip == 0)
			{
				status = executeCommand(cmd, arg, n);;
				break;
			}
		}
		
		i++;
		
		while(line[i] != '&' && line[i] != '|' && line[i] != ';' && line[i] != '\0')
		{
			if (size == n) 
			{
				n *= 2;
				arg = realloc(arg, (n));
			}
			if (line[i] != '"') 
			{
				strncat(arg, &line[i], 1);
				size++;
			}
			i++;
		}
		
		if (line[i] == '&')
		{
			if (skip == 0)
			{	
				status = executeCommand(cmd, arg, n);
				size = 0;
				if (status != 1) skip = 1;
			}
			
			i += 2;
			if (line[i] == '\0') break;
			cmd[0] = '\0';
			arg[0] = '\0';
		}
		else if (line[i] == '|')
		{
			if (skip == 0) 
			{
				status = executeCommand(cmd, arg, n);
				size = 0;
				if (status == 1) skip = 1;
			}
			
			i += 2;
			if (line[i] == '\0') break;
			cmd[0] = '\0';
			arg[0] = '\0';
		}
		else if (line[i] == ';')
		{
			if (skip == 0) status = executeCommand(cmd, arg, n);
			size = 0;
			i++;
			if (line[i] == '\0') break;
			cmd[0] = '\0';
			arg[0] = '\0';
			skip = 0;
		}
		else
		{
			if (skip == 0) status = executeCommand(cmd, arg, n);
			size = 0;
			cmd[0] = '\0';
			arg[0] = '\0';
		}
	}
	free(cmd);
	free(arg);
    if (status == 2) 
	{
		free(line);
		exit(0);
	}
}

int main(int argc, char **argv)
{
	char *input;
	while(1)
	{
		input = readCommand();
		if (!strcmp(input, "exit\n")) break;
		fineParse(input);
		free(input);
	}
	free(input);
	return EXIT_SUCCESS;
}
