#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 

/*
1.Read
2.Parse
3.Execute 
*/

#define TNYSH_RL_BUFSIZE 1024
#define TNYSHL_TOK_BUFSIZE 1024
#define TNYSH_TOK_DELIM " \t\r\n\a"

//definition of a Command. 
typedef struct {
    int commandsize; 
    char *command; 
    char **tokens; 
    int tokensize;  
} Command; 

//definition of a Shell 
typedef struct {
    char hostname[1024]; 
    Command **history;
    int history_size;
} Shell; 

//shell_init creates a Shell, and attempts to populate it with the host's name. 
Shell *shell_init() {
    Shell *usershell = malloc(sizeof(Shell)); 
    usershell->history = NULL; 
    usershell->history_size = 1; 

    //getting the host's name: 
    if(gethostname(usershell->hostname, sizeof(usershell->hostname)) != 0) {
        strncpy(usershell->hostname, "unknown", sizeof(usershell->hostname)); 
    }

    return usershell; 
}

//creates an empty instance of a command (for use in main)
Command *command_init() {
    Command *comm = malloc(sizeof(Command)); 
    comm->command = NULL; 
    comm->tokens = NULL; 
    comm->tokensize = NULL; 
    comm->commandsize = NULL; 
    return comm; 
}

//read_command(comm) consumes a command and reads it into a single line. (could've used getline but oh well)
void read_command(Command *comm) {
    comm->commandsize = TNYSH_RL_BUFSIZE; 
    comm->command = malloc(sizeof(char) * comm->commandsize);

    int c; 
    int position = 0; 

    //if we were unsuccesfully able to implement a command: 
    if(!comm->command) {
        fprintf(stderr, "tinysh: allocation erorr\n"); 
        exit(EXIT_FAILURE); 
    }

    //loop that will read from stdin until a newline of EOF is occured to get the newline. 
    while(1) {
        //read a character: 
        c = getchar(); 

        if(c == EOF || c == '\n') {
            comm->command[position] = '\0';
            return; 
        } else {
            comm->command[position] = c; 
        }
        position ++; 

        //reallocate if we have exceeded **the very large** command: 
        if(position >= comm->commandsize) {
            comm->commandsize += TNYSH_RL_BUFSIZE; 
            comm->command = realloc(comm->command, comm->commandsize); 
            if(!comm->command) {
                fprintf(stderr, "tinysh: allocation error \n"); 
                exit(EXIT_FAILURE); 
            }
        }
    }
}

//parse_command(com) consumes a Command and parses it into seperate "tokens" or "args". 
void parse_command(Command *com) {
    com->tokensize = TNYSHL_TOK_BUFSIZE;  

    com->tokens = malloc(com->tokensize * sizeof(char*)); 
    char *token; 

    if(!com->tokens) {
        fprintf(stderr, "tinysh: allocation error\n"); 
        exit(EXIT_FAILURE); 
    }

    //tokenisze the lined based on the delimeters that we have defined (see above)
    int position = 0; 
    token = strtok(com->command, TNYSH_TOK_DELIM); 
    while(token != NULL) {
        //adding to the tokens argument in the Command 
        com->tokens[position] = token; 
        position++; 

        if(position >= com->tokensize) {
            com->tokensize += TNYSHL_TOK_BUFSIZE; 
            com->tokens = realloc(com->tokens, com->tokensize * sizeof(char *)); 

            if(!com->tokens) {
                fprintf(stderr, "lsh: allocation error\n"); 
                exit(EXIT_FAILURE); 
            }
        }

        token = strtok(NULL, TNYSH_TOK_DELIM); 
    }
    com->tokens[position] = NULL; 
}

//main loop for execution of commands. 
void tinysh_loop(Command *com, Shell *sh) {
    int status; 
    do {
        printf("%s tinysh:> ", sh->hostname);
        read_command(com);
        parse_command(com); 
        status = tinysh_execute(com->tokens); 

        free(com->command); 
        free(com->tokens); 
    } while(status); 
}


int main(void) {
    Shell *sh = shell_init(); 
    Command *com = command_init(); 

    tinysh_loop(com, sh); 

    return EXIT_SUCCESS; 

    free(sh->history);
    free(com->command); 
    free(com->tokens); 
    free(sh);  
    free(com); 

}


