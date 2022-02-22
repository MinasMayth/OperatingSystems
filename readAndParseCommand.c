#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
        "cd",
        "help",
        "exit"
};
int (*builtin_func[]) (char **) = {
        &lsh_cd,
        &lsh_help,
        &lsh_exit
};

#define BUFFER_SIZE 1024

/*
 * User input is read in by initially allocating a buffer
 * size for input and then reallocating more space when needed.
 */
char *readInput(void) {
    int bufferSize = BUFFER_SIZE; //Buffer size Macro
    int position = 0;
    char *textBuffer = malloc(sizeof (char) * bufferSize);
    int character;

    if(!textBuffer){
        fprintf(stderr, "Text memory allocation error!\n");
        exit(EXIT_FAILURE);
    }
    while(1){

        character = getchar(); //get next character

        if(character == EOF || character == "\n"){ //EOF or newline signals end of input
            textBuffer[position] = "\0";
            return textBuffer;
        }else{
            textBuffer[position] = character;
        }
        position++;

        if(position >=bufferSize){ //Check if we need to allocate more space
            bufferSize += BUFFER_SIZE;
            textBuffer = realloc(textBuffer, bufferSize);
            if(!textBuffer){
                fprintf(stderr, "Text memory allocation error!\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define PARSE_BUFFER 64
#define PARSE_DELIM " \t\r\n\a"
char **parseInput(char *line) {
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
    return tokens;
}

int shell_launch(char **pString) {
    return 0;
}

int shell_num_builtins() {
    return 0;
}

int executeInput(char **args) {
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < shell_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return shell_launch(args);
}

/*
 * Main loop of shell. User input will be read in, split
 * into arguments and then executed.
 * */
void shell_loop() {
    char *userInput;
    char **arguments;
    int status;

    do {
        printf("> ");
        userInput = readInput();
        arguments = parseInput(userInput);
        status = executeInput(arguments);

        free(userInput);
        free(arguments);
    }while(status);
}

int main(int argc, char **argv) {
    // Load any configuration files

    //Main Shell Loop
    shell_loop();

    //Shutdown/Cleanup

    return EXIT_SUCCESS;
}
