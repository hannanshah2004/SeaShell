#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define MAX_SIZE_CMD 256
#define MAX_SIZE_ARG 16
#define MAX_HISTORY 10

char cmd[MAX_SIZE_CMD];
char *argv[MAX_SIZE_ARG];
pid_t pid;
char i;

char* history[MAX_HISTORY];
int history_index = 0;

void get_cmd();
void convert_cmd();
void c_shell();
void log_handle(int sig);

int main() {
    signal(SIGCHLD, log_handle);
    c_shell();
    return 0;
}

void c_shell() {
    while (1) {
        get_cmd();

        if (!strcmp("", cmd)) continue;

        if (!strcmp("exit", cmd)) break;

        if (!strncmp("cd", cmd, 2)) {
            char *dir = cmd + 3; // Assumes 'cd ' is the format

            if (!strcmp("..", dir)) {
                // Handle 'cd ..' to go back to the previous directory
                chdir("..");
            } else {
                // Regular 'cd' command
                if (chdir(dir) != 0) {
                    perror("cd failed");
                } else {
                    // Save the directory to history
                    if (history_index < MAX_HISTORY) {
                        history[history_index++] = strdup(dir);
                    } else {
                        free(history[0]);
                        for (int j = 0; j < MAX_HISTORY - 1; ++j) {
                            history[j] = history[j + 1];
                        }
                        history[MAX_HISTORY - 1] = strdup(dir);
                    }
                }
            }

            continue;
        }

        convert_cmd();

        pid = fork();
        if (-1 == pid) {
            printf("Failed to create a child\n");
        } else if (0 == pid) {
            execvp(argv[0], argv);
        } else {
            if (NULL == argv[i])
                waitpid(pid, NULL, 0);
        }
    }
}

void get_cmd() {
    printf("Shell>\t");
    fgets(cmd, MAX_SIZE_CMD, stdin);
    if ((strlen(cmd) > 0) && (cmd[strlen(cmd) - 1] == '\n'))
        cmd[strlen(cmd) - 1] = '\0';
}

void convert_cmd() {
    char *ptr;
    i = 0;
    ptr = strtok(cmd, " ");
    while (ptr != NULL) {
        argv[i] = ptr;
        i++;
        ptr = strtok(NULL, " ");
    }

    if (!strcmp("&", argv[i - 1])) {
        argv[i - 1] = NULL;
        argv[i] = "&";
    } else {
        argv[i] = NULL;
    }
}

void log_handle(int sig) {
    FILE *pFile;
    pFile = fopen("log.txt", "a");
    if (pFile == NULL) {
        perror("Error opening file.");
    } else {
        fprintf(pFile, "[LOG] Child process terminated.\n");
        fclose(pFile);
    }
}
