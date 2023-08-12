#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 100

struct shell {
    char hostname[1024];
}; 

void initShell(struct shell *usershell) {
    usershell->hostname[1023] = '\0'; 

    //getting the user's host information. 
    if(gethostname(usershell->hostname, 1023) != 0) {
        perror("gethostname"); 
        exit(EXIT_FAILURE);
    }  
}

// Function to parse the input line into command and arguments
void parse_line(char *line, char **args) {
    int i = 0;
    args[i] = strtok(line, " \t\n");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " \t\n");
    }
}



int main() {
    char *line = NULL;
    size_t len = 0;
    char *args[MAX_ARGS];

    struct shell usershell; 
    initShell(&usershell); 

    while (1) {
        printf("%s$-> ", usershell.hostname);
        getline(&line, &len, stdin);
        parse_line(line, args);

        if (args[0] == NULL) continue; // Empty command

        if (strcmp(args[0], "exit") == 0) {
            break;
        } else if (strcmp(args[0], "cd") == 0) {
            if (args[1] != NULL) {
                if (chdir(args[1]) != 0) {
                    perror("chdir");
                }
            }
        } else {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
            } else if (pid == 0) {
                if (execvp(args[0], args) < 0) {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            } else {
                wait(NULL);
            }
        }
    }

    free(line);
    return 0;
}
