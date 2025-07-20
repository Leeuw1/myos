#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <termios.h>

#include "readline/readline.h"
#include "readline/history.h"
#define vi_mode 0

#define MAX_COMMAND_LENGTH 63
#define BUFFER_SIZE	128
#define MAX_ARGC 8

static const char* PATH = "/bin/";
static struct termios saved_termios;

static char* prompt(void) {
	char cwd[64];
	memset(cwd, 0, 64);
	getcwd(cwd, 64);
	static char p[BUFFER_SIZE];
	snprintf(p, BUFFER_SIZE, "[pid=%d] %s $ ", (int)getpid(), cwd);
	return p;
}

static int cat(int argc, char** argv) {
	int fd = STDIN_FILENO;
	if (argc > 1) {
		fd = open(argv[1], O_RDONLY);
		if (fd == -1) {
			perror("cat");
			return EXIT_FAILURE;
		}
	}
	char buffer[BUFFER_SIZE];
	ssize_t size;
	while ((size = read(fd, buffer, BUFFER_SIZE)) > 0) {
		write(STDOUT_FILENO, buffer, size);
	}
	if (size == -1) {
		perror("cat");
		return EXIT_FAILURE;
	}
	close(fd);
	return EXIT_SUCCESS;
}

static int ls(int argc, char** argv) {
	const char* path = ".";
	if (argc > 1) {
		path = argv[1];
	}
	const int fd = open(path, O_RDONLY);
	if (fd == -1) {
		perror("ls");
		return EXIT_FAILURE;
	}
	struct stat status;
	fstat(fd, &status);
	if (S_ISREG(status.st_mode)) {
		printf("%s\n", path);
		return EXIT_SUCCESS;
	}
	const size_t dent_buffer_size = 64 * sizeof(struct posix_dent);
	void* buffer = malloc(dent_buffer_size);
	ssize_t size = posix_getdents(fd, buffer, dent_buffer_size, 0);
	close(fd);
	if (size < 0) {
		perror("ls");
		free(buffer);
		return EXIT_FAILURE;
	}
	struct posix_dent* entry = buffer;
	for (; (void*)entry < buffer + size; ++entry) {
		printf("%s  ", entry->d_name);
	}
	putchar('\n');
	free(buffer);
	return EXIT_SUCCESS;
}

static int compare_int(const void* left, const void* right) {
	return *(const int*)left - *(const int*)right;
}

static int sort(int argc, char** argv) {
	if (argc <= 1) {
		return EXIT_SUCCESS;
	}
	int* arr = malloc((argc - 1) * sizeof *arr);
	for (int i = 1; i < argc; ++i) {
		arr[i - 1] = atoi(argv[i]);
	}
	qsort(arr, argc - 1, sizeof(*arr), compare_int);
	for (int i = 0; i < argc - 1; ++i) {
		printf("%d ", arr[i]);
	}
	putchar('\n');
	free(arr);
	return EXIT_SUCCESS;
}

static int parse_and_exec_command(char* command, size_t command_length) {
	int argc = 0;
	char* argv[MAX_ARGC + 1] = {};
	char* arg = command;
	for (size_t i = 0; i < command_length; ++i) {
		if (argc == MAX_ARGC) {
			break;
		}
		if (command[i] == ' ') {
			command[i] = '\0';
			// Actual size of buffer is MAX_COMMAND_LENGTH + 1
			if (command[i + 1] != ' ' && command[i + 1] != '\0') {
				argv[argc++] = arg;
				arg = &command[i + 1];
			}
		}
	}
	if (arg < command + command_length && argc < MAX_ARGC) {
		argv[argc++] = arg;
	}
	argv[argc] = NULL;
	// Check for builtin functions
	if (strcmp(argv[0], "cd") == 0) {
		if (chdir(argc < 2 ? "/" : argv[1])) {
			perror("chdir");
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}
	if (strcmp(argv[0], "sort") == 0) {
		return sort(argc, argv);
	}
	if (strcmp(argv[0], "ls") == 0) {
		return ls(argc, argv);
	}
	if (strcmp(argv[0], "cat") == 0) {
		return cat(argc, argv);
	}
	if (strcmp(argv[0], "mkdir") == 0) {
		if (argc > 1) {
			if (mkdir(argv[1], 0) == -1) {
				perror("mkdir");
				return EXIT_FAILURE;
			}
		}
		return EXIT_SUCCESS;
	}
	if (strcmp(argv[0], "rmdir") == 0) {
		if (argc > 1) {
			if (rmdir(argv[1]) == -1) {
				perror("rmdir");
				return EXIT_FAILURE;
			}
		}
		return EXIT_SUCCESS;
	}
	if (strcmp(argv[0], "mv") == 0) {
		if (argc > 2) {
			if (rename(argv[1], argv[2]) == -1) {
				perror("mv");
				return EXIT_FAILURE;
			}
		}
		return EXIT_SUCCESS;
	}
	if (strcmp(argv[0], "rm") == 0) {
		if (argc > 1) {
			if (unlink(argv[1]) == -1) {
				perror("rm");
				return EXIT_FAILURE;
			}
		}
		return EXIT_SUCCESS;
	}
	if (strcmp(argv[0], "exit") == 0) {
		if (getpid() != 1) {
			exit(EXIT_SUCCESS);
		}
		return EXIT_FAILURE;
	}
	if (strcmp(argv[0], "echo") == 0) {
		for (int i = 1; i < argc; ++i) {
			printf("%s ", argv[i]);
		}
		putchar('\n');
		return EXIT_SUCCESS;
	}
	if (strcmp(argv[0], "clear") == 0) {
		printf("\x1b[2J\x1b[1;1H");
		return EXIT_SUCCESS;
	}
	const size_t path_length = strlen(PATH) + strlen(argv[0]);
	char* path = malloc(path_length + 1);
	path[0] = '\0';
	if (argv[0][0] != '/' && argv[0][0] != '.') {
		strcpy(path, PATH);
	}
	strcat(path, argv[0]);
	path[path_length] = '\0';

	tcgetattr(STDIN_FILENO, &saved_termios);

	const pid_t pid = fork();
	if (pid == 0) {
		printf("[shell] Running program '%s'...\n", path);
		char* envp[] = { NULL };
		execve(path, argv, envp);
		perror("execve");
		abort();
	}
	free(path);
	int status;
	waitpid(pid, &status, 0);
	// Restore terminal settings which might get changed by child process
	tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios);
	return status;
}

static int repl(void) {
	rl_filename_rewrite_hook = NULL;
	rl_editing_mode = vi_mode;
	volatile int running = 1;
	while (running) {
		char* command = readline(prompt());
		if (command == NULL) {
			continue;
		}
		if (command[0] == '\0') {
			continue;
		}
		parse_and_exec_command(command, strlen(command));
		add_history(command);
		free(command);
	}
	return EXIT_SUCCESS;
}

int main(int argc, const char* argv[]) {
	if (argc < 2) {
		return repl();
	}
	if (argc == 2) {
		return EXIT_FAILURE;
	}
	if (strcmp(argv[1], "-c") != 0) {
		return EXIT_FAILURE;
	}
	const size_t command_length = strlen(argv[2]);
	if (command_length > MAX_COMMAND_LENGTH) {
		return EXIT_FAILURE;
	}
	char command[MAX_COMMAND_LENGTH + 1];
	strcpy(command, argv[2]);
	return parse_and_exec_command(command, command_length);
}
