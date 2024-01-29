#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdbool.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <time.h>
#include <dirent.h>

#define MAX_INPUT_SIZE 1024 // Expected maximum input size
#define MAX_TOKENS 100 // Expected maximum number of tokens
char* redirect; // Holds the redirect value(">" or ">>" or ">>>")
FILE* aliasFile; // File which holds aliases
char lastExecutedCommand[MAX_INPUT_SIZE] = "No command executed yet!"; // Last executed command for bello

void printPrompt() { // Prints prompt
    char hostName[256];
    char *userName;
    char currentDir[PATH_MAX];

    // Get the host name
    if (gethostname(hostName, sizeof(hostName)) != 0) {
        perror("Error getting host name");
        exit(EXIT_FAILURE);
    }

    // Get the user name
    userName = getenv("USER");
    if (userName == NULL) {
        perror("Error getting user name");
        exit(EXIT_FAILURE);
    }

    // Get the current directory
    if (getcwd(currentDir, sizeof(currentDir)) == NULL) {
        perror("Error getting current directory");
        exit(EXIT_FAILURE);
    }

    // Print the prompt
    printf("%s@%s %s%s ", userName, hostName, currentDir, " --- ");
}

int parseInput(char *input, char **tokens, int *background, char **outputFile) { // Parses input
    int tokenCount = 0; // number of tokens
    char *token = (char*) malloc(MAX_INPUT_SIZE); // single token
    char tokenAlias[MAX_INPUT_SIZE]; // used to handle quotation marks as a one token
    const char *delimiter = " \t\n"; // parse from delimeter

    while ((token = strsep(&input, delimiter)) != NULL && tokenCount < MAX_TOKENS) {
        if (*token != '\0') {
            if (strcmp(token, "&") == 0) { // if & exists, then it is background process
                *background = 1;
                continue;
            }
            else if (token[0] == '\"'){ // this block is to handle quotation marks
                if(token[strlen(token) - 1] == '\"'){
                    token[strlen(token) - 1] = 0;
                    token = token + 1;
                    tokens[tokenCount] = token;
                    tokenCount++;
                    continue;
                }
                strcpy(tokenAlias, token + 1);
                char* tempToken;
                tempToken = strsep(&input, delimiter);
                while(tempToken != NULL && tempToken[strlen(tempToken) - 1] != '\"'){
                    if(tempToken[0] == '\"'){
                        strcat(tokenAlias, " ");
                        strcat(tokenAlias, tempToken);
                        tempToken = strsep(&input, delimiter);
                        while(tempToken != NULL && tempToken[strlen(tempToken) - 1] != '\"'){
                            strcat(tokenAlias, " ");
                            strcat(tokenAlias, tempToken);
                            tempToken = strsep(&input, delimiter);
                        }
                    }
                    strcat(tokenAlias, " ");
                    strcat(tokenAlias, tempToken);
                    tempToken = strsep(&input, delimiter);
                }
                if(tempToken != NULL){
                    tempToken[strlen(tempToken) - 1] = 0;
                    strcat(tokenAlias, " ");
                    strcat(tokenAlias, tempToken);
                }    
                else{
                    token[strlen(token) - 1] = 0;
                    token = token + 1;
                    strcpy(tokenAlias, token);
                }
                strcpy(token, tokenAlias);
            } 
            else if (strcmp(token, ">") == 0 || strcmp(token, ">>") == 0 || strcmp(token, ">>>") == 0) { // if there is a redirection open a outputfile
                redirect = token;
                token = strsep(&input, delimiter);  // Get the next token as the output file
                *outputFile = token;
                // skip the output file token in the regular tokens array
                continue;
            }

            tokens[tokenCount] = token;
            tokenCount++;
        }
    }

    tokens[tokenCount] = NULL; // Null-terminate the array of tokens since execvp expects last element of args NULL
    free(token);
    return tokenCount;
}

void bello(){ // bello function
    // get user name
    char *userName = getenv("USER");
    // get host name
    char hostName[256];
    gethostname(hostName, sizeof(hostName));
    // get TTY
    char *tty = ttyname(0);
    // get shell name
    char *shellName = getenv("SHELL");
    // get home location
    char *homeLoc = getenv("HOME");
    // get the current time
    time_t currentTime;
    time(&currentTime);
    // get number of processes
    DIR *dir;
    struct dirent *entry;
    int processCount = 0;

    // Open the "/proc" directory
    dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // Count the number of directories in "/proc," each representing a process
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            // Check if the entry name is a numeric value (representing a process ID)
            char *endptr;
            strtol(entry->d_name, &endptr, 10);
            if (*endptr == '\0') {
                processCount++;
            }
        }
    }

    closedir(dir);

    printf("%s\n%s\n%s\n%s\n%s\n%s\n%s%d\n", userName, hostName, lastExecutedCommand, tty, shellName, homeLoc, ctime(&currentTime), processCount);
    return;
    
}

void executeCommand(char **tokens, int background, char *outputFile, int tokenCount) { // execute commands
    pid_t pid = fork();

    if (pid < 0) { // if pid is negative, raise an error
        perror("fork");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) { // Child process

        if (outputFile != NULL) { // if there is an output file, handle redirections
            int flags = (strcmp(redirect, ">>") == 0) ? O_WRONLY | O_CREAT | O_APPEND : 
                         (strcmp(redirect, ">>>") == 0) ? O_WRONLY | O_CREAT | O_APPEND : 
                         O_WRONLY | O_CREAT | O_TRUNC;  
            
            int fd = open(outputFile, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (fd == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }

            int pipefd[2];

            if (strcmp(redirect, ">>>") == 0) { // to handle ">>>"
                // Create a pipe for communication between parent and child
                if (pipe(pipefd) == -1) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                pid_t subpid = fork();

                if (subpid < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } 
                else if (subpid == 0) { // Child process (to reverse the output)
                    close(pipefd[0]); // Close the read end of the pipe

                    // Redirect standard output to the write end of the pipe
                    if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                        perror("dup2");
                        close(pipefd[1]);
                        exit(EXIT_FAILURE);
                    }

                    close(pipefd[1]);

                    // Execute the command
                     if(strcmp(tokens[0], "bello") == 0){
                        bello();
                        exit(EXIT_SUCCESS);
                    }
                    else{
                        execvp(tokens[0], tokens);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }  
                } 
                else { // Parent process
                    close(pipefd[1]); // Close the write end of the pipe

                    // Redirect standard input to the read end of the pipe
                    if (dup2(pipefd[0], STDIN_FILENO) == -1) {
                        perror("dup2");
                        close(pipefd[0]);
                        exit(EXIT_FAILURE);
                    }

                    close(pipefd[0]);

                    // Redirect standard output to the file
                    int filefd = open(outputFile, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    if (filefd == -1) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }

                    // Redirect standard output to the file
                    if (dup2(filefd, STDOUT_FILENO) == -1) {
                        perror("dup2");
                        close(filefd);
                        exit(EXIT_FAILURE);
                    }

                    close(filefd);

                    // Reverse the content before writing to the file
                    execlp("rev", "rev", (char *)NULL);
                    perror("execlp");
                    exit(EXIT_FAILURE);
                }

            } 
            else {
                // Redirect standard output to the file
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    close(fd);
                    exit(EXIT_FAILURE);
                }

                close(fd);

                // Execute the command
                if(strcmp(tokens[0], "bello") == 0){
                    bello();
                    exit(EXIT_SUCCESS);
                }
                else{
                    execvp(tokens[0], tokens);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }    
            }
        }         
        else if(strcmp(tokens[0], "bello") == 0){
            bello();
        }
        else {
            execvp(tokens[0], tokens);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } 
    else { // Parent process
        if (!background) {
            // Wait for the child process to complete
            waitpid(pid, NULL, 0);

            // Handle termination of background processes here
            int status;
            waitpid(-1, &status, WNOHANG);
        }
    }
}

int main() {
    char input[MAX_INPUT_SIZE]; // input saved here
    char *tokens[MAX_TOKENS]; // tokens array

    while (true) {
        // Print the prompt
        printPrompt();

        // Get user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("Error reading input");
            exit(EXIT_FAILURE);
        }

        // Remove the newline character from the input
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // Skip processing if the input is empty
        if (strlen(input) == 0) {
            continue;
        }

        // Exit the shell if the user enters "exit"
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Hold last executed command here
        char inputBeforeParse[MAX_INPUT_SIZE];
        strcpy(inputBeforeParse, input);

        // Parse the input
        int background = 0;
        char *outputFile = NULL;
        int tokenCount = parseInput(input, tokens, &background, &outputFile);

        // If the command is a alias creation add it to the alias file
        if (strcmp(tokens[0], "alias") == 0) {
            char string[MAX_INPUT_SIZE];
            char* first = string;
            strcpy(first, tokens[1]);
            strcat(first, " ");
            char string2[MAX_INPUT_SIZE];
            char* second = string2;
            strcpy(second, tokens[3]);
            strcat(first, second);
            aliasFile = fopen("aliasList.txt", "a");
            fprintf(aliasFile, "%s\n", first);
            fclose(aliasFile);
        }
        
        // Execute the command
        else {
            char line[MAX_INPUT_SIZE];
            // if there is an alias file, search the file to see if a command from alias file is executed
            if (aliasFile = fopen("aliasList.txt", "r")) {
                while (fgets(line, sizeof(line), aliasFile) != NULL) {
                    char* firstSpace = strchr(line, ' ');
                    *firstSpace = '\0';
                    if (strcmp(line, tokens[0]) == 0) {
                        char newInput[MAX_INPUT_SIZE]; // to add flags to the aliases
                        strcpy(newInput, firstSpace + 1);
                        newInput[strlen(newInput) - 1] = 0;
                        for(int i = 1; i < tokenCount; i++){
                            strcat(newInput, " ");
                            strcat(newInput, tokens[i]);
                        }
                        tokenCount = parseInput(newInput, tokens, &background, &outputFile);
                        break;
                    }
                }
                fclose(aliasFile);
            }
            //if not from alias file execute command via this function
            executeCommand(tokens, background, outputFile, tokenCount);
        }

        // Copy input to the last executed command
        strcpy(lastExecutedCommand, inputBeforeParse);

        // Empty tokens array
        *tokens = NULL;
    }  

    return 0;
}
