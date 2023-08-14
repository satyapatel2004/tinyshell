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


//forward declarations of the shell's builtin commands: 
int tinysh_cd(Command *com); 
int tinysh_help(Command *com); 
int tinysh_exit(Command *com); 


// a list of the tinyshell built in functions. 
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
}; 

// an array of pointers to the builtin functions in tinyshell. 
int (*builtin_func[]) (Command *com) = {
    &tinysh_cd,
    &tinysh_help,
    &tinysh_exit
};

//returns the number of builtins
int tinysh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *); 
}

/*** builtin function implementations ***/
int tinysh_cd(Command *com) {
    char **args = com->tokens; 
    if(args[1] == NULL) {
        fprintf(stderr, "tinysh: expected argument to \"cd\"\n"); 
    } else {
        if(chdir(args[1]) != 0) {
            perror("tinysh"); 
        }
    }
    return 1; 
}

//will output a helpful help page which will display information about tinyshell. 
int tinysh_help(Command *com) {

    printf("Satya Patel's Tiny Shell\n"); 
    printf("Type program names and arguments, and hit enter.\n"); 
    printf("The following are built in: \n"); 

    int num_built = tinysh_num_builtins(); 

    for(int i = 0; i < num_built; i ++) {
        printf(" %s\n", builtin_str[i]); 
    }

    printf("Use the man command for information on other programs. \n");
    return 1; 

}

//implementation for the lsh_exit function. 
int tinysh_exit(Command *com) {
    return 0; 
}


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
    comm->tokensize = 0; 
    comm->commandsize = 0; 
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
                fprintf(stderr, "tinysh: allocation error\n"); 
                exit(EXIT_FAILURE); 
            }
        }

        token = strtok(NULL, TNYSH_TOK_DELIM); 
    }
    com->tokens[position] = NULL; 
}

//tinysh_launch will launch external commands that are not implemented. 
int tinysh_launch(Command *com) {
    pid_t pid, wpid; 
    char **args = com->tokens; 
    int status; 

    // Starting a new process (and saving the return value in "pid")
    pid = fork(); 
    if(pid < 0) {
        // Error in forking
        perror("tinysh");
        return 1;
    }

    if(pid == 0) {
        // Child process
        if(execvp(args[0], args) == -1) {
            // If an error with the child process
            perror("tinysh"); 
            exit(EXIT_FAILURE);
        }
        // The child should exit here if execvp fails
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED); 
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); 
    }

    return 1; 
}

//will execute the commands. 
int tinysh_execute(Command *com) {
    int i; 
    char **args = com->tokens; 

    //if there are no args, there is nothing to execute. 
    if(args[0] == NULL) {
        return 1; 
    }

    //getting the number of builtins that we have (so we don't have to call the function in the loop)
    int num_builtins = tinysh_num_builtins(); 

    //try to see if our arguments are a builtin command. 
    for(i = 0; i < num_builtins; i ++) {
        //seeing if the first argument (the command we want to execute is one of the builtins)
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(com); 
        }
    }

    //if not a builtin, then launch an external command. 
    return tinysh_launch(com); 
}


//main loop for execution of commands. 
void tinysh_loop(Command *com, Shell *sh) {
    int status; 
    do {
        printf("%s tinysh:> ", sh->hostname);
        read_command(com);
        parse_command(com); 
        status = tinysh_execute(com); 

        free(com->command); 
        com->command = NULL; 
        free(com->tokens); 
        com->tokens = NULL; 
    } while(status); 
}


void destroy(Command *com, Shell *sh) {
    free(sh->history);
    free(com->command); 
    free(com->tokens); 
    free(sh);  
    free(com); 
}

int main(void) {
    Shell *sh = shell_init(); 
    
    //the com will be written over each time by tinysh_loop 
    Command *com = command_init(); 

    tinysh_loop(com, sh); 

    //freeing the allocated memory. 
    destroy(com, sh); 

    return EXIT_SUCCESS; 

}


