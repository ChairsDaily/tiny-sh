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

#define TOKEN_BUFSIZE       64
#define ALLOC_ERROR         "allocation error\n"
#define EXEC_ERROR          "execution error\n"
#define TOKEN_DELIM         " \t\r\n\a"
#define RUNNING             1

#define PROGRAM_STRING         "Tiny.sh Version 0.1"
#define COPY_STRING            "Copyleft (C) 2020 Kaleb H."

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
char** tsh_tokenize (char* line) {

    int bufsize = TOKEN_BUFSIZE;
    int location = 0;
    char** tokens = malloc(sizeof(char*) * bufsize);
    char* token;

    token = strtok(line, TOKEN_DELIM);
    while (token != NULL) {
        tokens[location] = token; 
        location++;

        if (location >= bufsize) {
            bufsize += TOKEN_BUFSIZE;
            tokens = tsh_realloc(tokens, bufsize);
        }
        token = strtok(NULL, TOKEN_DELIM);
    }
    tokens[location] = NULL;
    return tokens;
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
        if (execvp(args[0], args) == -1) {
            fprintf(stderr, EXEC_ERROR);
        }
        exit(-1);

    } else {
        waitpid(pid, &status, 0);
    }

    return 0;
}

void tsh_cd (char** args) {
    if (args[1] == NULL) {
        fprintf(stderr, EXEC_ERROR); exit(-1);
    } else {
        if (chdir(args[1]) != 0) {
            perror("tsh");
        }
    }
}

int main (int argc, char** argv) {

    puts (PROGRAM_STRING);
    puts (COPY_STRING);
    puts ("Ctr.Z to Exit\n");

    while (RUNNING) {

        char* input = readline("~ ");
        if (strcmp(input, "") == 0) {continue;}

        add_history(input);
        char** tokens = tsh_tokenize(input);

        if (strcmp(tokens[0], "cd") == 0) {
            tsh_cd(tokens);
        } else {tsh_execute(tokens);}

        free(input);
        free(tokens);
    }
}

