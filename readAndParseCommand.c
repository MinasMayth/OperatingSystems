#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

struct commandList{
    int numberOfCommands;
    char** operators;
    char* command[][3];
};

struct argumentsContainer{
    char **arguments;
    int size;
};

void printCommands(struct commandList list) {
    printf("%d commands\n", list.numberOfCommands);
    printf("%s\n", list.command[0][0]);
    printf("%s\n", list.command[0][1]);
    //printf("%s\n", list.command[0][2]);


}

void fineParserV3(struct commandList result, char **arguments, int n) {
    result.numberOfCommands = 1;

    for(int i=0; i<n; i++){
        char *current_arg = arguments[i];
        if(i == 0){
            result.command[0][0] = current_arg;
            printf("%s\n", result.command[0][0]);
        }else{
            result.command[0][1] = current_arg;
            result.command[0][2] = NULL;
        }
    }
    printCommands(result);

}

struct commandList fineParserV2(char **arguments, int n) {
    struct commandList result;
    result.numberOfCommands = 0;
    result.operators = malloc(64*sizeof(char**));
    int commandIterator = 0;
    char temp[1000];
    bool newArgument = true;

    for(int i=0; i<n; i++){
        char *current_arg = arguments[i];
        if(newArgument){
            result.command[commandIterator][0] = current_arg;
            printf("%s", result.command[commandIterator][0]);
            newArgument = false;
        }else if (current_arg[0] == '"'){
            //Start of arguments
            strcat(temp, current_arg);
        }else if (i == n-1){
            strcat(temp, current_arg);
            strcpy(result.command[commandIterator][1], temp);
            //printf("%s", result.command[commandIterator][1]);
            result.command[commandIterator][2] = NULL;
            result.numberOfCommands++;
        }else{
            strcat(temp, current_arg);
        }
    }
    printCommands(result);
    return result;
}

/*
 * Fine parsing function to format commands
printf("%s", list.command[0][0]) * into a style that the shell executor can
 * understand.
 */
struct commandList fineParser(char **arguments, int n) {
    struct commandList result;
    int commandIterator = 0;
    char *temp = malloc(sizeof(arguments[0]));
    unsigned long tempSize = 0;
    bool newArgument = true;

    for(int i=0; i<n; i++){
        char *current_arg = arguments[i];
        if(newArgument){
            result.command[commandIterator][0] = current_arg;
            printf("%s", current_arg);
            newArgument = false;
        }else if (current_arg[0] == "&" || current_arg[0] == "|"){ //We still need to check for semicolon
            //We have reached the end of a command
            newArgument = true;
            result.command[commandIterator][1] = temp;
            result.command[commandIterator][2] = NULL;
            result.numberOfCommands++;
            commandIterator++;
            free(temp);
            temp = malloc(0);
            tempSize = 0;
        }else if (current_arg[0] == '"'){
            //Start of arguments
            tempSize += strlen(current_arg);
            temp = realloc(temp, tempSize* sizeof(char*));
            strcat(temp, current_arg);
        }else if (i == n-1){
            tempSize += strlen(current_arg);
            temp = realloc(temp, tempSize* sizeof(char*));
            strcat(temp, current_arg);
            result.command[commandIterator][1] = temp;
            result.command[commandIterator][2] = NULL;
            result.numberOfCommands++;

        }else{
            tempSize += strlen(current_arg);
            realloc(temp, tempSize* sizeof(char*));
            strcat(temp, current_arg);
        }

    }

    free(temp);
    return result;
}

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

/*
 * User input is read in by initially allocating a buffer
 * size for input and then reallocating more space when needed.
 */
char *readInput(void) {
    char *userInput = NULL;
    ssize_t bufferSize = 0; //getline will allocate the buffer

    if (getline(&userInput, &bufferSize, stdin) == -1){
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);  // EOF
        } else  {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }

    return userInput;
}

#define PARSE_BUFFER 64
#define PARSE_DELIM " \t\r\n\a"
struct argumentsContainer parseInput(char *line) {
    int bufferSize = PARSE_BUFFER, position = 0;
    char **tokens = malloc(bufferSize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "shell: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, PARSE_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufferSize) {
            bufferSize += PARSE_BUFFER;
            tokens = realloc(tokens, bufferSize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "shell: memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, PARSE_DELIM);
    }
    tokens[position] = NULL;

    struct argumentsContainer result;
    result.arguments = tokens;
    result.size = position;

    return result;
}

/*
 * Main loop of shell. User input will be read in, split
 * into arguments and then executed.
 * */
void shell_loop() {
    char *userInput;
    struct argumentsContainer arguments;
    int status;

    do {
        printf("> ");
        userInput = readInput();
        arguments = parseInput(userInput);

        struct commandList toExecute;

        fineParserV3(toExecute, arguments.arguments, arguments.size);

        //printf("%d", toExecute.numberOfCommands);
        printCommands(toExecute);
        //executeCommand(toExecute.command, toExecute.numberOfCommands, toExecute.operators);


        free(userInput);
        free(arguments.arguments);
    }while(status);
}


int main(int argc, char **argv) {
    // Load any configuration files

    //Main Shell Loop
    shell_loop();

    //Shutdown/Cleanup

    return EXIT_SUCCESS;
}
