#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 512

int main() {
	int pipe1[2], pipe2[2];
    char filepath[MAX_INPUT];
    char str_[MAX_INPUT];
    char error[MAX_INPUT];

	printf("File name: ");
	fgets(filepath, MAX_INPUT, stdin);
	filepath[strlen(filepath) - 1] = 0;

	int file = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (file == -1) {
		perror("file open");
		exit(1);
	}

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1){
   		perror("pipe");
        exit(1);
    }

    switch (fork()){
    	case -1:
        	perror("fork");
            exit(1);
        case 0:
        	dup2(pipe1[0], STDIN_FILENO);
            dup2(pipe2[1], STDERR_FILENO);
    		dup2(file, STDOUT_FILENO);

    		close(file);
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            execlp("./build/child", "child", NULL);
            perror("execlp");
            exit(1);
        default:
        	close(pipe1[0]);
            close(pipe2[1]);

            while (1){
            	printf("String or 'e' for exit: ");
                fgets(str_, MAX_INPUT, stdin);
                if (strncmp(str_, "e", 1) == 0) break;
                write(pipe1[1], str_, strlen(str_));
            }
            close(pipe1[1]);

            while(read(pipe2[0], error, MAX_INPUT) > 0) {
              	printf("error: %s \n", error);
            }
            close(pipe2[0]);
            wait(NULL);
    }
    return 0;
}
