#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <string.h>
#include<time.h>

#define MAX_INPUT_LENGTH 100

char *readInput();
int executeBuiltInCommands(char *command,char **args,char* path);
void parseInput(char *input,char **command, char*** arguments);
void writeToConsole();
void bello();


int main()
{
    char *input = NULL;
    char *path = getenv("PATH");
    char *command = NULL;
    char **arguments = NULL;
    int validCommand;
    while (1)
    {
        writeToConsole();
        input = readInput();
        if (*input == '\0') // newline case check
        {
            free(input);
            continue;
        }
        if (strcmp(input, "exit") == 0)  // exit condition
        {
            printf("Exiting shell\n");
            free(input);
            break;
        }
        if (strcmp(input, "bello") == 0)  // exit condition
        {
            bello();
            free(input);
            continue;
        }
        parseInput(input,&command,&arguments);  // parse the input into tokens and classify them
        if (command != NULL)
        {
            validCommand = executeBuiltInCommands(command, arguments,path);
        }
        free(command);  // memory cleaning
        for (int i = 0; arguments[i] != NULL; ++i)
        {
            free(arguments[i]);
        }
        free(arguments);
        free(input);

    }
    return 0;
}


void writeToConsole()
{
    struct passwd *pw = getpwuid(getuid());
    char host[MAX_INPUT_LENGTH];
    char cwd[MAX_INPUT_LENGTH];
    gethostname(host,sizeof(host));
    getcwd(cwd,sizeof(cwd));
    char *home = pw->pw_dir;
    int homeLength = strlen(home);
    if (strncmp(cwd, home, homeLength) == 0)
    {
        printf("%s@%s ~%s --- ", pw->pw_name, host, cwd + homeLength);
    }
    else
    {
        printf("%s@%s %s --- ", pw->pw_name, host, cwd);
    }
}


char *readInput()
{
    char line[MAX_INPUT_LENGTH];
    char *input = (char*) malloc(MAX_INPUT_LENGTH);
    if (fgets(line, sizeof(line), stdin) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        if (input != NULL)
        {
            strcpy(input, line);
        }
        else
        {
            perror("Memory allocation error");
        }
    }
    else
    {
        perror("Error while reading input");
    }
    return input;
}


void parseInput(char *input,char **command, char*** arguments)
{
    char *token = strtok(input," ");
    *command = strdup(token);  // first token +must be command
    int count = 0;
    *arguments = (char **)malloc(sizeof(char*));  // rest should be arguments
    while ((token = strtok(NULL, " ")) != NULL)
    {
        *arguments = (char **)realloc(*arguments, (count + 1) * sizeof(char *));
        (*arguments)[count++] = strdup(token);
    }
    *arguments = (char **)realloc(*arguments, (count + 1) * sizeof(char *));
    (*arguments)[count] = NULL;
}


int executeBuiltInCommands(char *command,char **args,char* path)
{
    int flag = 0;
    char *copy = strdup(path);
    char *pathDirectories = strtok(copy, ":");
    while (pathDirectories != NULL)
    {
        char currentPath[MAX_INPUT_LENGTH * 4];
        snprintf(currentPath, MAX_INPUT_LENGTH, "%s/%s", pathDirectories, command);
        if (access(currentPath, X_OK) == 0)
        {
            pid_t pid = fork();
            if (pid < 0)  // error cas
            {
                perror("Forking error");
            }
            else if (pid == 0)  // child process case
            {
                char *allArgs[2 + MAX_INPUT_LENGTH];
                allArgs[0] = command;
                int i;
                for (i = 0; args[i] != NULL; ++i)
                {
                    allArgs[i + 1] = args[i];
                }
                allArgs[i + 1] = NULL;

                execv(currentPath, allArgs);

                // execv failed if we reach this point
                perror("Exec error");
                exit(EXIT_FAILURE);
            }
            else  // parent process, wait for child
            {
                waitpid(pid, NULL, 0);
                return 1;
            }
        }
         pathDirectories = strtok(NULL, ":");  // look for next directory
    }
    free(copy);
    return 0;
}


void bello()
{
    struct passwd *pw = getpwuid(getuid());  // username
    char host[MAX_INPUT_LENGTH];
    gethostname(host,sizeof(host));  // hostname
    // TODO: last command
    char *tty = ttyname(STDIN_FILENO);  // tty
    char *shellName = getenv("SHELL");  // shell
    char *home = pw->pw_dir;  // home
    time_t t;
    time(&t); // time
    int numProcesses = system("ps aux | awk 'NR > 1' | wc -l");
    printf("1. Username: %s\n", pw->pw_name);
    printf("2. Hostname: %s\n", host);
    //printf("3. Last Executed Command: %s\n", lastCommandResult);
    printf("4. TTY: %s\n", tty);
    printf("5. Current Shell Name: %s\n", shellName);
    printf("6. Home Location: %s\n", home);
    printf("7. Current Time and Date: %s", ctime(&t));
    // TODO: buraya dikkat sikinti cikarabilir
    printf("8. Current number of processes being executed: %d\n", numProcesses);
}
