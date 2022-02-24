#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>
/*
 * Command list container used by the shell
 * for execution.
 */
struct commandList{
    int numberOfCommands;
    char** operators;
    char* command[][3];
};
/*
 * Arguments container used by the parser
 * to convert a line of user input to a format
 * that is accepted by the shell executor.
 */
struct argumentsContainer{
    char **arguments;
    int size;
};

/*
 * function to pad a string with a certain number of spaces.
 */
void pad(char *destination, int numOfSpaces, int size)
{
    int len = strlen(destination);

    if(len + numOfSpaces >= size ) {
        numOfSpaces = size - len - 1;
    }
    memset(destination + len, ' ', numOfSpaces );
    destination[len + numOfSpaces] = '\0';

}

/*
 * Function to search a string for a specific character,
 * and then to remove all occurrences of that character from the string.
 */
void removeChar(char str[], char t )
{
    int i,j;
    i = 0;
    while(i<strlen(str))
    {
        if (str[i]==t)
        {
            for (j=i; j<strlen(str); j++)
                str[j]=str[j+1];
        } else i++;
    }
}

/*
 * The heart of the fineParser. This function will accept an array of arguments
 * and a destination array, formatting everything nicely so that it can be run
 * by the shell.
 */
void fineParserSingular(char **arguments, int size, struct commandList result, int i1) {
    result.command[i1][0] = arguments[0];
    if(size == 2){
        removeChar(arguments[1], '"');
        result.command[i1][1] = arguments[1];
    }else{
        char temp[100] = "";
        for(int i=1;i<size;i++){
            strcat(temp, arguments[i]);
            if(i != size-1){
                pad(temp, 1, 100);
            }

        }
        removeChar(temp, '"');
        result.command[i1][1] = malloc((sizeof(temp)/sizeof(temp[0]))*sizeof(char*));
        strcpy(result.command[i1][1], temp);

    }

    result.command[i1][2] = NULL;
}


/*
 * Function to sort an array in ascending order. A second array will be sorted
 * in the same order.
 */
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

        if(skip)
        {
            indexOfCommand++;
            skip = 0;
        }

        nextCommand = command[indexOfCommand];
        if ((nextCommand[0] == "exit")) exit(0);

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
                if((operators[indexOfCommand] == "&&")) skip = 1; //command failed and skip next
            }
            else
            {
                if ((operators[indexOfCommand] == "||")) skip = 1; //command succes but skip anyway
            }
        }
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
/*
 * Code-specific function that searches through a string for all occurrences of a certain operand and then adds
 * it to the operandsFound array. The indexes are also stored in a separate array for sorting later.
 */
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
/*
 * Counts the occurrences of a certain substring in a string
 */
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


/*
 * function to separate the input argument into singular words,
 * signified by the POSIX delimiters below.
 */
#define PARSE_BUFFER 64
#define PARSE_DELIM " \t\r\a"
struct argumentsContainer parseInput(char *line) {
    int bufferSize = PARSE_BUFFER, position = 0;
    char **text_tokens = malloc(bufferSize * sizeof(char*));
    char *text_token;

    if (!text_tokens) {
        fprintf(stderr, "shell: memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    text_token = strtok(line, PARSE_DELIM);
    while (text_token != NULL) {
        text_tokens[position] = text_token;
        position++;

        if (position >= bufferSize) {
            bufferSize += PARSE_BUFFER;
            text_tokens = realloc(text_tokens, bufferSize * sizeof(char*));
            if (!text_tokens) {
                fprintf(stderr, "shell: memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        text_token = strtok(NULL, PARSE_DELIM);
    }
    text_tokens[position] = NULL;

    struct argumentsContainer result;
    result.arguments = text_tokens;
    result.size = position;

    return result;
}

/*
 * Function to split a line of user input into separate commands. Functionally very similar to
 * parseInput but uses different delimiters.
 */
#define ARG_BUFFER 64
#define ARG_DELIM ";&&||\n"
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
 * Function to count the number of operands in a line of input
 */
int count_operands(char *input) {
    int amperCount = get_substr_count(input, "&&");
    int semiCount = get_substr_count(input, ";");
    int lineCount = get_substr_count(input, "||");
    int newlineCount = get_substr_count(input, "\n");

    return amperCount+semiCount+lineCount+newlineCount;
}
/*
 * Extracts command arguments from the argumentsContainer and formats them to fit
 * with the commandList format.
 */
void extract_arguments(struct argumentsContainer arguments, struct commandList finalCommands) {
    fineParserSingular(arguments.arguments, arguments.size, finalCommands, 0);


    finalCommands.numberOfCommands = 1;

    char* operators[] = {"x"};

}
/*
 * Function to extract given operands from a line of user input and store
 * them in the finalCommands.operators. A max size of 10 operands is set.
 */
void extract_operands(char *userInput, struct commandList finalCommands) {
    char* operandsFound[10];
    int operandsIndex[10];
    int position = 0;

    position = collectOperands(userInput, "&&", operandsFound, operandsIndex, position);
    position = collectOperands(userInput, ";", operandsFound, operandsIndex, position);
    position = collectOperands(userInput, "||", operandsFound, operandsIndex, position);
    position = collectOperands(userInput, "\n", operandsFound, operandsIndex, position);


    arraySort(operandsIndex, operandsFound, position);

    finalCommands.operators = operandsFound;

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
        struct commandList finalCommands;
        finalCommands.numberOfCommands = 0;

        printf("> ");
        userInput = readInput();

        int operandsCount = count_operands(userInput);

        if(operandsCount != 0){ //Multiple commands entered

            extract_operands(userInput, finalCommands);

            arguments = splitArguments(userInput);

            for(int i=0; i<arguments.size; i++){
                struct argumentsContainer commandArgs = parseInput(arguments.arguments[i]);


                fineParserSingular(commandArgs.arguments, commandArgs.size, finalCommands, i);

                finalCommands.numberOfCommands++;

                free(commandArgs.arguments);

            }
            executeCommand(finalCommands.command, finalCommands.numberOfCommands, finalCommands.operators);

        }else{
            arguments = parseInput(userInput);

            if(arguments.size==1){ //single word command
                finalCommands.command[0][0] = arguments.arguments[0];
                finalCommands.command[0][1] = NULL;
                finalCommands.command[0][2] = NULL;
                finalCommands.numberOfCommands = 1;

                char* operators[] = {"x"}; //garbage value

                executeCommand(finalCommands.command, finalCommands.numberOfCommands, operators);

            }else if(arguments.size>1){
                extract_arguments(arguments, finalCommands);
                char* operators[] = {"x"};
                executeCommand(finalCommands.command, finalCommands.numberOfCommands, operators);

            }

        }

        free(userInput);
        free(arguments.arguments);

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
