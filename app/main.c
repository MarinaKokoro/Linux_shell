#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdint.h>
#include <fcntl.h>


void handle_sighup(int sig){
	printf("Configuration reloaded \n");
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
		char s2[10];
		strncpy(s2, input, 8);
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
		else if (strcmp(s2, "\\l /dev/") == 0){
			const char *path = (input+3);
			FILE* disk = fopen(path, "rb");
			if (disk == NULL){
				printf("No such file or directory");
			}
			if(fseek(disk, 510, SEEK_SET) != 0){
				printf("Ошибка при смещении \n");
				fclose(disk);
			}
			uint8_t signature[2];
			if(fread(signature, 1, 2, disk) != 2){
				printf("Ошибка при чтении \n");
				fclose(disk);
			}
			fclose(disk);

			if(signature[0] == 0x55 && signature[1] == 0xAA){
				printf("Диск загрузочный \n");
			}
			else{
				printf("Диск не загрузочный \n");
			}
		}
	       	else if(strcmp(s, "\\mem") == 0){
			pid_t pid = atoi(input+4); 
			const char* output_file = "damp_proc";
			char maps_path[256], mem_path[256], line[256];
			snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);
			snprintf(mem_path, sizeof(mem_path), "/proc/%d/mem", pid);

			FILE* maps_file = fopen(maps_path, "r");
			if (!maps_file) {
			    perror("Cannot open maps \n");
			}

			int mem_fd = open(mem_path, O_RDONLY);
			if (mem_fd == -1) {
			    perror("Cannot open mem \n");
			    fclose(maps_file);
			}

			int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (out_fd == -1) {
			    perror("Cannot open output file \n");
			    fclose(maps_file);
			    close(mem_fd);
			}

			// Читаем регионы памяти из maps и дампим их
			while (fgets(line, sizeof(line), maps_file)) {
			    unsigned long start, end;
			    if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
			        char buffer[4096];
			        for (unsigned long addr = start; addr < end; addr += sizeof(buffer)) {
			            ssize_t bytes = pread(mem_fd, buffer, sizeof(buffer), addr);
			            if (bytes > 0)
			                write(out_fd, buffer, bytes);
				}
			    }
			}

			fclose(maps_file);
			close(mem_fd);
			close(out_fd);

			printf("Dump saved to %s\n", output_file);
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
