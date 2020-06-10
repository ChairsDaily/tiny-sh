/*
*    Simple UNIX shell with one builtin for changing directory.
*    Copyleft (C) 2020  Kaleb Horvath
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <editline/readline.h>
#include <editline/history.h>
#include "tinysh.h"

/*
* Reallocate token buffer
* @param buffer the original block of memory
* @param bufsize the new allocation size
* @return resized buffer
*/
char** tsh_realloc (char** buffer, int bufsize) {
    bufsize *= sizeof(char*);
    buffer = realloc(buffer, bufsize);

    if (!buffer) {
        fprintf(stderr, ALLOC_ERROR);
        exit(-1);
    }

    return buffer;
}

/*
* Tokenize a line from standard in (taken from editline)
* @param line of text taken from stdin
* @return a buffer of tokens ready for execvp
*/
char** tsh_tokenize (char* str) {

    int size = TOKEN_BUFSIZE;
    int loc = 0;
    char** words = malloc(sizeof(char*) * size);
    char* ptr;

    ptr = strtok(str, TOKEN_DELIM);
    while (ptr != NULL) {
        words[loc] = ptr; 
        loc++;

        if (loc >= size) {
            size += TOKEN_BUFSIZE;
            words = tsh_realloc(words, size);
        }
        ptr = strtok(NULL, TOKEN_DELIM);
    }
    words[loc] = NULL;
    return words;
}


/*
* Fork, exec, and wait for child process
* @param command line arguments
* @return zero if successful
* raises EXEC_ERROR if something whent wrong
*/
int tsh_execute (char** args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        // the child process will land here
        // execvp will replace forked copy with program
        // in arguments
        if (execvp(args[0], args) == -1) {
            fprintf(stderr, EXEC_ERROR);
        }
        exit(-1);

    } else if (pid == -1) {
        fprintf(stderr, EXEC_ERROR);
        exit(-1);
    } else {
        // parent process lands here
        // forces parent to wait for child
        waitpid(pid, &status, 0);
    }

    return SHELL_STATUS;
}

int tsh_cd (char** args) {
    if (args[1] == NULL) {
        fprintf(stderr, EXEC_ERROR); exit(-1);
    } else {
        if (chdir(args[1]) != 0) {
            perror("tsh");
        }
    }
    return SHELL_STATUS;
}

int tsh_export (char** args) {

    if (args[1] == NULL) {

        fprintf(stderr, EXEC_ERROR);
        exit(-1);
    } else {
        // env memory block is process-local
        // so this only affects us and our children
        if (setenv(args[1], args[2], 1) != 0)
            perror("tsh");
    }
    return SHELL_STATUS;
}

int tsh_import (char** args) {

    char* value;
    
    if (args[1] == NULL) {

        fprintf(stderr, EXEC_ERROR);
        exit(-1);
    } else {
        value = getenv(args[1]);
        if (value == NULL) {
            fprintf(stderr, EXEC_ERROR);
        } else {
            printf("%s\n", value);
        }
    }
    return SHELL_STATUS;
}

int main (int argc, char** argv) {

    puts (PROGRAM_STRING);
    puts (COPY_STRING);
    puts ("Ctr.Z to Exit\n");

    while (!SHELL_STATUS) {

        char* input = readline("~ ");
        if (strcmp(input, "") == 0) {continue;}

        add_history(input);
        char** tokens = tsh_tokenize(input);

        /*
        int i = 0;
        while (tokens[i] != '\0') {
            printf("%s\n", tokens[i]);
            i++;
        }
        */

        if (strcmp(tokens[0], "cd") == 0) {
            tsh_cd(tokens);

        } else if (strcmp(tokens[0], "set") == 0) {
            tsh_export(tokens);

        } else if (strcmp(tokens[0], "get") == 0) {
            tsh_import(tokens);

        } else {tsh_execute(tokens);}

        free(input);
        free(tokens);
    }
}

