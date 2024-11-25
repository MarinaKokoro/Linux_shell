#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

void handle_sighup(int sig){
	printf("Configuration reloaded");
}

int main() {
	
	printf("PID: %d\n", getpid());
	FILE *history = fopen("history.txt", "a");

	if(signal(SIGHUP, handle_sighup) == SIG_ERR){
		perror("Не удалось установить обработчик сигнала");	
	}

	while(1){
		printf("$ ");
  		fflush(stdout);

  		char input[100];
  		if(fgets(input, 100, stdin) == NULL){
			fclose(history);
			return 0;
		}
  		input[strlen(input) - 1] = '\0';
		fputs(input, history);
       		fputs("\n", history);

		char s[100];
		strncpy(s, input, 4);
		if(strcmp(s,"echo") == 0){
			strncpy(s, input + 5, 94);
			printf("%s\n", s);
		}
		else if(strcmp(s,"\\e $") == 0){
			const char *name = (input+4);
			const char *env_p = getenv(name);
    			printf("%s = %s\n", name, env_p);
		}
		else if(strcmp(s, "/bin") == 0) {
			const char *name = (input+4);
			pid_t pid = fork();
			if(pid < 0){
				printf("ERROR fork\n");
			}
			else if (pid == 0){
				execl(input, name, NULL);
				perror("ERROR execl\n");
			}
			else{
				wait(NULL);
			}
                }
		char s2[100];
		strncpy(s2, input, 8);
		else if (strcmp(s2, "\\l /dev/") == 0){
			const char *path(input+3);
			FILE* disk = fopen(path, "rb");
			if (disk == NULL){
				printf("No such file or directory");
			}
		} 


		else if(strcmp(input, "exit") == 0 || strcmp(input, "\\q") == 0){
			fclose(history);
			return 0;
		}
		else{
  			printf("%s: command not found\n", input);
		}
	}
}
