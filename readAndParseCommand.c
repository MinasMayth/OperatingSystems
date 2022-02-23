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

 void fineParserSingular(char **arguments, int size,  char* result[0][3]) {
    result[0][0] = arguments[0];
    result[0][1] = arguments[1];
    result[0][2] = NULL;
}


// Function to perform Selection Sort
void arraySort(int arr[], char* arr2[], int n)
{
     int temp;
     char* temp2;
    //Sort the array in ascending order
    for (int i = 0; i < n; i++) {
        for (int j = i+1; j < n; j++) {
            if(arr[i] > arr[j]) {
                temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;

                temp2 = arr2[i];
                arr2[i] = arr2[j];
                arr2[j] = temp2;
            }
        }
    }
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

int collectOperands(const char * haystack, const char *needle, char** operandsFound, int* operandsIndex, int position)
{
    int count = 0;
    const char *tmp = haystack;
    while( (tmp = strstr( tmp, needle))){
        ++count;
        tmp++;
        operandsFound[position] = needle;
        operandsIndex[position] = (int)(tmp-haystack);
        position++;
    }
    return position;
}

int get_substr_count(const char * haystack, const char *needle)
{
    int count = 0;
    const char *tmp = haystack;
    while( (tmp = strstr( tmp, needle))){
        ++count;
        tmp++;
    }
    return count;
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

#define ARG_BUFFER 64
#define ARG_DELIM ";&&||"
struct argumentsContainer splitArguments(char *line) {
    int bufferSize = ARG_BUFFER, position = 0;
    char **tokens = malloc(bufferSize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "shell: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, ARG_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufferSize) {
            bufferSize += ARG_BUFFER;
            tokens = realloc(tokens, bufferSize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "shell: memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, ARG_DELIM);
    }
    tokens[position] = NULL;

    struct argumentsContainer result;
    result.arguments = tokens;
    result.size = position;

    return result;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
/*
 * Main loop of shell. User input will be read in, split
 * into arguments and then executed.
 * */
void shell_loop() {
    char *userInput;
    struct argumentsContainer arguments;
    int status;


    do {
        struct commandList finalCommands;

        printf("> ");
        userInput = readInput();

        int amperCount = get_substr_count(userInput, "&&");
        int semiCount = get_substr_count(userInput, ";");
        int lineCount = get_substr_count(userInput, "||");

        const int operandsCount = amperCount+semiCount+lineCount;

        if(operandsCount != 0){
            //struct operandInfo operandsList[operandsCount];

            char* operandsFound[10];
            int operandsIndex[10];
            int position = 0;

            position = collectOperands(userInput, "&&", operandsFound, operandsIndex, position);
            position = collectOperands(userInput, ";", operandsFound, operandsIndex, position);
            position = collectOperands(userInput, "||", operandsFound, operandsIndex, position);

            arraySort(operandsIndex, operandsFound, operandsCount);

            finalCommands.operators = operandsFound;

            arguments = splitArguments(userInput);

            for(int i=0; i<arguments.size; i++){
                //printf("%s\n", arguments.arguments[i]);
                struct argumentsContainer commandArgs = parseInput(arguments.arguments[i]);

                static char* toExecute[0][3];
                fineParserSingular(commandArgs.arguments, commandArgs.size, toExecute);


                for(int j = 0; j<3; j++){
                    finalCommands.command[i][j] = toExecute[0][j];
                }
                finalCommands.numberOfCommands++;

                //printf("%s\n", finalCommands.command[i][1]);
                //free(commandArgs.arguments);

            }
            executeCommand(finalCommands.command, finalCommands.numberOfCommands, finalCommands.operators);

        }else{
            arguments = parseInput(userInput);

            static char* toExecute[0][3];
            fineParserSingular(arguments.arguments, arguments.size, toExecute);


            for(int i = 0; i<3; i++){
                finalCommands.command[0][i] = toExecute[0][i];
            }
            finalCommands.numberOfCommands = 1;


            //char* operators[] = {"||", "&&", ";"};
            //printf("%d", toExecute.numberOfCommands);
            //printCommands(toExecute);
            //char* command[][3] = {{"echo", "a", NULL}, {"echo", "a", NULL}, {"echo", "c", NULL}, {"echo", "d", NULL}};;
            //char* operators[] = {"||", "&&", ";"};

            //char* command[][3] = {{"./b.out", "< in > out", NULL}, {"echo", "a", NULL}, {"echo", "b", NULL} };
            //char* operators[] = {"||",";"};

            //executeCommand(command, 3, operators);
            executeCommand(finalCommands.command, finalCommands.numberOfCommands, finalCommands.operators);


        }



        free(userInput);
        //free(arguments.arguments);
        //free(finalCommands.command);

    }while(true);
}
#pragma clang diagnostic pop


int main(int argc, char **argv) {
    // Load any configuration files

    //Main Shell Loop
    shell_loop();

    //Shutdown/Cleanup

    return EXIT_SUCCESS;
}
