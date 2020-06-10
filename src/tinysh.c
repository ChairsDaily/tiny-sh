#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <editline/readline.h>
#include <editline/history.h>

#define BUFSIZE         10
#define ALLOC_ERROR     "allocation error\n"
#define EXEC_ERROR      "execution error\n"
#define ARG_ERROR       "argument error\n"
#define DELIMS          " \t\r\n\a "

int tsh_cd(char **args);
int tsh_export(char **args);
int tsh_import(char **args);

char* builtins[] = {
    "cd", "get", "set"
};
int (*builtin_funcs[]) (char**) = {
    &tsh_cd, 
    &tsh_import, 
    &tsh_export
};

void tsh_error_handle (char* error) {
    fprintf(stderr, error);

    if (error != EXEC_ERROR) {
        exit(-1);
    }
}

/* Reallocate heap memory for array of strings
*  (used in tokenizer if the user input is too large)
*  @param buffer the original allocation
*  @param new_size the new amount of bytes to allocate
*  @return pointer to the start of newly allocated space
*/
char** tsh_realloc (char** buffer, int new_size) {
    
    new_size *= sizeof(char*);
    buffer = realloc(buffer, new_size);

    // malloc can fail if you use up more process
    // memory than is allowed by OS
    if (!buffer) {
        tsh_error_handle(ALLOC_ERROR);
    }
    return buffer;
}

/* Split a shell command into tokens to be passed to execvp
*  @param input string, must be non-static and not in .text
*          (heap allocd works, or reading from stdin, but not char[])
*  @return pointer to the start of the allocated heap space for the
*          array of strings
*/
char** tsh_splitcmd (char* str) {

    // allocate space for array of strings
    // which will hold result after we split
    int size = BUFSIZE;
    char** tokens = malloc(sizeof(char*) * size);
    char* curr_token;
    int location = 0;

    // as long as strtok was able to split on delim
    // continue copying token into tokens buffer
    curr_token = strtok(str, DELIMS);
    while (curr_token != NULL) {
        tokens[location] = curr_token;
        location++;

        if (location >= size) {

            size += BUFSIZE;
            tokens = tsh_realloc(tokens, size);
        }
        curr_token = strtok(NULL, DELIMS);
    }
    // NULL terminate the array of strings
    // so our builtins know when the arguments
    // run out 
    tokens[location] = NULL;
    return tokens;
}

/* Execute a command that can be found in PATH
*  (this introduces the need to modify the environment
*   block of our processes memory)
*  @param arguments the result of tsh_tokenize over a non-static character array
*  @return 0 on success, raises ARGS/EXEC_ERROR otherwise
*/
int tsh_execute (char** arguments) {
    
    // not unsigned int cuz they told me so >>.>>
    pid_t pid;
    int status_lock;

    // null check the arguments, 0 should never be NULL
    if (arguments[0] == NULL) {
        tsh_error_handle(ARG_ERROR);
    }

    // will copy our process
    // we now have to handle the child context
    // and the parent context
    pid = fork();
    if (pid == 0) {
        
        // this is the child, so lets replace
        // it with the program to run
        // using exec system call
        // (note on success this will redirect its output to stdout)
        if (execvp(arguments[0], arguments) == -1) {

            // will exit out our child process not this shell instance
            tsh_error_handle(EXEC_ERROR);
        }
    } else if (pid == -1) {

        // this means that there was an error forking
        // our process 
        tsh_error_handle(EXEC_ERROR);
    } else {
        // this is where our parent process lands
        // lets wait for the child to finish
        // note we have to point it to the status lock
        waitpid(pid, &status_lock, 0);
    }
    return 0; // UNIX success
}

/*
* Here we implement our three builtins
* the main loop should NULL check pointers 0-2
* and raise ARG_ERROR if it hits one
* before any of these run
*/
int tsh_cd (char** arguments) {
    if (chdir(arguments[1]) != 0)
        perror("tsh");

    return 0;
}

int tsh_import (char** arguments) {

    // this because getenv does not write to stdout
    char* value = getenv(arguments[1]);
    if (value == NULL) {
        tsh_error_handle(EXEC_ERROR);
    } else {

        // auto prints until '\0' termination
        // auto derefs the pointer as well
        /* 
            without auto dereffing, youd have to do the following
            (where p is a pointer to heap memory for a character array
            and s is a string literal)
            
            memcpy(p, s, strlen(s) + 1);
            while (*p != '\0') {
                printf("%c", *p);   // deref to get val@addr
                p++;                // incr address
            }
        */
        printf("%s\n", value);
    }
    return 0; 
}

int tsh_export (char** arguments) {

    if (setenv(arguments[1], arguments[2], 1) != 0)
        tsh_error_handle(EXEC_ERROR);

    return 0;
}

int main (int argc, char** argv) {

    // env memory block is process-local
    // so this only affects us and outinysh
    puts ("FREE SOFTWARE");
    puts ("ctrl+z to Exit\n");

    while (1) {

        char* input = readline("~ ");
        if (strcmp(input, "") == 0) {continue;}

        add_history(input);
        char** tokens = tsh_splitcmd(input);
        int did_exec = 0;

        for (int i = 0; i < 3; i++) {
            if (strcmp(tokens[0], builtins[i]) == 0) {

                (*builtin_funcs[i])(tokens);
                did_exec = 1;
            }
        }
        if (!did_exec) tsh_execute(tokens);

        free(input);
        free(tokens);
    }
}
