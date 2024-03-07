#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h> // Added for signal handling

#define MAX 100
#define LOGS "/home/muhammad_wael/os/lab1/logs.log"

typedef enum {
    builtin,
    executable
} input_t;

typedef enum {
    cd,
    echo,
    export
} command_t;

void setup_environment();
void on_child_exit(int signal); // Updated prototype for signal handler
void reap_child_zombie();
void write_to_log_file(char line[]);
void register_child_signal(void (*on_child_exit)(int));
void shell(void);
command_t check_builtin_cmd(char* arg);
void execute_shell_builtin(char* argv[]);
void execute_command(char* argv[]);
input_t check_input(char* arg);
void parse_input(char cmd[], char* argv[]);
void evaluate_expression(char* argv[]);
void parse_arguments(char* args[], char* argv[]);
void execute_cd(char* argv[]);
void execute_export(char* argv[]);
void execute_echo(char* argv[]);

int main() {
    register_child_signal(on_child_exit);
    setup_environment();
    shell();
    return 0;
}

void register_child_signal(void (*on_child_exit)(int)) {
    signal(SIGCHLD, on_child_exit);
}

void setup_environment() {
    chdir(getenv("HOME")); // Changed to set initial directory to home directory
}

void on_child_exit(int signal) {
    reap_child_zombie();
    write_to_log_file("Child terminated\n");
}

void reap_child_zombie() {
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0);
}

void write_to_log_file(char line[]) {
    FILE* log;

    log = fopen(LOGS, "a");
    if (log == NULL)
        return;

    fprintf(log, "%s", line);
    fclose(log);
}

void shell(void) {
    char cmd[MAX];
    while (1) {
        printf("muhammad_wael:~$ ");
        fgets(cmd, MAX, stdin);
        cmd[strcspn(cmd, "\n")] = 0; // Remove newline character from input
        if (strlen(cmd) == 0) {
            continue;
        }
        char* argv[MAX] = { NULL };
        parse_input(cmd, argv);
        evaluate_expression(argv);
        if (!argv[0] || strcmp(argv[0], "exit") == 0) {
            break;
        }
        input_t input = check_input(argv[0]);
        switch (input) {
        case builtin:
            execute_shell_builtin(argv);
            break;
        case executable:
            execute_command(argv);
            break;
        }
    }
}

command_t check_builtin_cmd(char* arg) {
    command_t val;
    if (strcmp(arg, "cd") == 0)
        val = cd;
    else if (strcmp(arg, "echo") == 0)
        val = echo;
    else if (strcmp(arg, "export") == 0)
        val = export;
    return val;
}

void execute_shell_builtin(char* argv[]) {
    command_t cmd;
    cmd = check_builtin_cmd(argv[0]);

    switch (cmd) {
    case cd:
        execute_cd(argv);
        break;
    case echo:
        execute_echo(argv);
        break;
    case export:
        execute_export(argv);
        break;
    }
}

void execute_command(char* argv[]) {
    pid_t child = fork();

    if (child < 0) {
        perror("Forking Error");
    }
    else if (child == 0) {
        char* args[MAX] = { NULL };

        parse_arguments(args, argv);
        execvp(args[0], args);

        perror("Execvp Error");
        exit(1);
    }
    else {
        waitpid(child, 0, 0);
    }
}

input_t check_input(char* arg) {
    input_t val;
    if (strcmp(arg, "cd") == 0 || strcmp(arg, "echo") == 0 || strcmp(arg, "export") == 0)
        val = builtin;
    else
        val = executable;
    return val;
}

void parse_input(char cmd[], char* argv[]) {
    int argc = 0;
    char* arg = strtok(cmd, " ");

    while (argc < MAX && arg != NULL) {
        if (arg[0] == '\"') {  // Check if the argument starts with a double quote
            // Concatenate with the next token until the closing quote is found
            while (arg[strlen(arg) - 1] != '\"') {
                char* next_token = strtok(NULL, "\"");
                if (next_token != NULL) {
                    strcat(arg, " ");
                    strcat(arg, next_token);
                } else {
                    printf("Error: Unclosed quote.\n");
                    break;  // Break if there's no more token
                }
            }
            if (arg[strlen(arg) - 1] == '\"') {
                arg[strlen(arg) - 1] = '\0'; // Remove the closing quote
            }
        }
        argv[argc++] = arg;
        arg = strtok(NULL, " ");
    }
}



void evaluate_expression(char* argv[]) {
    for (int i = 1; argv[i] != NULL; i++) {
        if (argv[i][0] == '$') {
            char* env_var = getenv(argv[i] + 1);
            if (env_var != NULL)
                argv[i] = env_var;
        }
    }
}

void parse_arguments(char* args[], char* argv[]) {
    int argc = 0;

    for (int i = 0; argv[i] != NULL; i++) {
        args[argc++] = argv[i];
    }
}

void execute_cd(char* argv[]) {
    if (chdir(argv[1]) != 0) {
        perror("Error changing directory\n");
    }
}

void execute_echo(char* argv[]) {
    for (int i = 1; argv[i] != NULL; i++) {
        printf("%s", argv[i]);
        if (argv[i + 1] != NULL) {
            printf(" ");
        }
    }
    printf("\n");
}


void execute_export(char* argv[]) {
    char* value;
    char* identifier;

    for (int i = 1; argv[i] != NULL; i++) {
        char* arg = argv[i];
        if (arg[0] == '\"') { // Check if the argument starts with a double quote
            char* end_quote = strchr(arg + 1, '\"'); // Find the position of the closing quote
            if (end_quote != NULL) {
                *end_quote = '\0'; // Null-terminate to separate identifier and value
                identifier = arg + 1; // Start from the character after the opening quote
                value = end_quote + 1; // Start from the character after the closing quote
                setenv(identifier, value, 1); // Set the environment variable
            } else {
                printf("Error: Unclosed quote.\n");
            }
        } else if ((value = strchr(arg, '=')) != NULL) {
            *value = '\0';
            identifier = arg;
            value++;
            setenv(identifier, value, 1);
        }
    }
}








