#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#define MAX_INPUT_LENGTH 128
#define ALIASES_PATH "aliases.txt"

char *readInput();
int executeBuiltInCommands(char *command, char **args, char *path, int background, int redirecting, int clearFile, char *outputFile, int reRedirecting);
void parseInput(char *input, char **command, char ***arguments, int *redirecting, int *clearFile, char **outputFile);
void writeToConsole();
void bello();
void trim(char *str);
void divideString(const char *input, int index, char **firstPart, char **secondPart);
int reRedirect(char *input, char *command, char **args, char *path, int background, int redirecting, int clearFile, char *outputFile);
void reverse(char *str, ssize_t num);
void removeQuote(char *str);
void createAlias(char *aliasName, char *aliasCommand);
void writeAliases();
void loadAliases();
int findAlias(char *name, char **command);
void freeAliases();
void printAliases();
void parseAlias(char **arguments);
void killZombies();

typedef struct // struct for aliases. Keeps alias name and command
{
    char *aliasName;
    char *aliasCommand;
} Alias;

// global variables
Alias *aliases;
int aliasCount;
int numberOfExecutedProcesses = 0;
char *lastExecutedCommand = NULL;

int main()
{
    char *input = NULL;
    char *path = getenv("PATH");
    char *command = NULL;
    char **arguments = NULL;
    char *outputFile = NULL;
    // bool variables
    int validCommand;
    int background;
    int redirecting;
    int clearFile;
    int reRedirecting;
    aliasCount = 0;
    loadAliases();
    while (1)
    {
        killZombies();
        redirecting = 0;
        clearFile = 0;
        writeToConsole();
        input = readInput();
        trim(input);

        if (input[strlen(input) - 1] == '&')
        {
            background = 1;
            input[strlen(input) - 1] = '\0';
            trim(input);
        }
        else
            background = 0;

        if (*input == '\0') // newline case check
        {
            free(input);
            continue;
        }
        if (strcmp(input, "exit") == 0) // exit condition
        {
            writeAliases();
            free(input);
            break;
        }
        if (strcmp(input, "bello") == 0) // bello case
        {
            bello();
            free(input);
            continue;
        }
        reRedirecting = reRedirect(input, command, arguments, path, background, redirecting, clearFile, outputFile);
        if (!reRedirecting)
        {
            parseInput(input, &command, &arguments, &redirecting, &clearFile, &outputFile);
            if (strcmp(command, "alias") == 0)
            {
                lastExecutedCommand = strdup(input);
                parseAlias(arguments);
                free(input);
                free(command); // memory cleaning
                for (int i = 0; arguments[i] != NULL; ++i)
                {
                    free(arguments[i]);
                }
                free(arguments);
                continue;
                // memory free
            }

            // parse the input into tokens and classify them
            validCommand = executeBuiltInCommands(command, arguments, path, background, redirecting, clearFile, outputFile, 0);
            free(command); // memory cleaning
            for (int i = 0; arguments[i] != NULL; ++i)
            {
                free(arguments[i]);
            }
            free(arguments);
        }

        if (!validCommand)
        {
            printf("Command is not found\n");
        }
        else
            lastExecutedCommand = strdup(input);

        free(input);
    }
    return 0;
}

void writeToConsole() // function to  write necessary info to console
{
    struct passwd *pw = getpwuid(getuid());
    char host[MAX_INPUT_LENGTH];
    char cwd[MAX_INPUT_LENGTH];
    gethostname(host, sizeof(host));
    getcwd(cwd, sizeof(cwd));
    char *home = pw->pw_dir;
    int homeLength = strlen(home);
    if (strncmp(cwd, home, homeLength) == 0)
        printf("%s@%s ~%s --- ", pw->pw_name, host, cwd + homeLength);
    else
        printf("%s@%s %s --- ", pw->pw_name, host, cwd);
}

char *readInput() // reads input from terminal
{
    char line[MAX_INPUT_LENGTH];
    char *input = (char *)malloc(MAX_INPUT_LENGTH);
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

void trim(char *str) // removes leading and trailing spaces
{
    while (isspace(*str))
    {
        str++;
    }
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
    {
        end--;
    }
    *(end + 1) = '\0';
}

void removeQuote(char *str) // removes quotes for aliasing
{
    int len = strlen(str);
    memmove(str, str + 1, len - 2);
    str[len - 2] = '\0';
}

void divideString(const char *input, int index, char **firstPart, char **secondPart) // helper function for aliasing
{
    *firstPart = strndup(input, index - 2);
    *secondPart = strdup(input + index - 2);
}

void reverse(char *str, ssize_t num) // helper function for >>>
{
    for (int i = 0; i < num / 2; i++)
    {
        char temp = str[i];
        str[i] = str[num - i - 1];
        str[num - i - 1] = temp;
    }
}

void killZombies() // kills finished background processes
{
    pid_t terminatedPId;
    int status;
    while (terminatedPId = waitpid(-1, &status, WNOHANG) > 0)
    {
        numberOfExecutedProcesses--;
    }
}

void parseInput(char *input, char **command, char ***arguments, int *redirecting, int *clearFile, char **outputFile)
{ // input parser. Sets necessary values to parameters
    char *copiedInput = strdup(input);
    char *token = strtok(copiedInput, " ");
    *command = strdup(token); // first token must be command
    int count = 0;
    *arguments = (char **)malloc(sizeof(char *)); // rest should be arguments
    while ((token = strtok(NULL, " ")) != NULL)
    {
        if (strcmp(token, ">") == 0)
        {
            token = strtok(NULL, " ");
            *outputFile = strdup(token);
            *redirecting = 1;
            *clearFile = 1;
            break;
        }
        else if (strcmp(token, ">>") == 0)
        {
            token = strtok(NULL, " ");
            *outputFile = strdup(token);
            *redirecting = 1;
            *clearFile = 0;
            break;
        }
        *arguments = (char **)realloc(*arguments, (count + 1) * sizeof(char *));
        (*arguments)[count++] = strdup(token);
    }
    *arguments = (char **)realloc(*arguments, (count + 1) * sizeof(char *));
    (*arguments)[count] = NULL;
    free(copiedInput);
}

void parseAlias(char **arguments) // specific parser for aliases since they have a unique form
{
    int totalLength = 1;
    for (int i = 2; arguments[i] != NULL; ++i)
    {
        totalLength += strlen(arguments[i]) + 1; // +1 for the space between arguments
    }

    char *mergedCommand = (char *)malloc(totalLength);
    if (mergedCommand != NULL)
    {
        strcpy(mergedCommand, arguments[2]);
        for (int i = 3; arguments[i] != NULL; ++i)
        {
            strcat(mergedCommand, " "); // Add space between arguments
            strcat(mergedCommand, arguments[i]);
        }
    }
    removeQuote(mergedCommand);
    createAlias(arguments[0], mergedCommand);
    free(mergedCommand);
}

int executeBuiltInCommands(char *command, char **args, char *path, int background, int redirecting, int clearFile, char *outputFile, int reRedirecting)
{ // executes commands. fork - exec logic is implemented here
    // returns 1 for successfull execution, otherwise returns 0
    int isBello = 0;
    int isAlias = findAlias(command, &command); // at first, check if the command is alias or not
    if (isAlias)                                // if it is an alias, look for what it stands for then parse the command again
        parseInput(command, &command, &args, &redirecting, &clearFile, &outputFile);

    char *copy = strdup(path);
    char *pathDirectories = strtok(copy, ":");
    while (pathDirectories != NULL) // look for builtin commands in the path
    {
        char currentPath[MAX_INPUT_LENGTH * 6];
        snprintf(currentPath, MAX_INPUT_LENGTH, "%s/%s", pathDirectories, command);
        if (access(currentPath, X_OK) == 0 || strcmp(command, "bello") == 0)
        { // if the path is accessible or the command is bello, start execution
            int rfd[2];
            pipe(rfd); // message passing will be used for >>>
            pid_t pid = fork();
            numberOfExecutedProcesses++;
            if (pid < 0) // error case
            {
                perror("Forking error");
            }
            else if (pid == 0) // child process case
            {
                if (redirecting) // > or >> cases
                {
                    int fd;
                    if (!clearFile)
                        fd = open(outputFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    else
                        fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                    if (fd == -1)
                    {
                        perror("Error opening output file");
                        exit(EXIT_FAILURE);
                    }

                    // Redirect standard output to the opened file
                    if (dup2(fd, STDOUT_FILENO) == -1)
                    {
                        perror("Error redirecting output");
                        close(fd);
                        exit(EXIT_FAILURE);
                    }

                    // Close the original file descriptor
                    close(fd);
                }
                if (reRedirecting) // >>> cases
                {
                    close(rfd[0]);
                    dup2(rfd[1], STDOUT_FILENO);
                    close(rfd[1]);
                }

                if (background) // & cases
                    setpgid(0, 0);

                if (strcmp(command, "bello") == 0) // bello cases
                {
                    isBello = 1;
                    bello();
                    free(copy); // free the memory allocated by strdup
                    exit(0);    // indicates that the command was executed
                }
                char *allArgs[2 + MAX_INPUT_LENGTH];
                allArgs[0] = command;
                int i;
                for (i = 0; args[i] != NULL && strcmp(args[i], "&") != 0; ++i)
                {
                    allArgs[i + 1] = args[i];
                }
                allArgs[i + 1] = NULL;
                execv(currentPath, allArgs); // wont go further if execv is successfull
                perror("execv failed");
                return 0;
            }
            else // parent process, wait for child
            {
                if (!background) // if background, do not wait
                {
                    int res = waitpid(pid, NULL, 0);
                    if (res > 0)
                        numberOfExecutedProcesses--;
                }

                if (reRedirecting) // reverse the output and write
                {
                    close(rfd[1]);
                    char str[MAX_INPUT_LENGTH * 16];
                    ssize_t number = read(rfd[0], str, MAX_INPUT_LENGTH * 16);
                    if (number > 0)
                    {
                        reverse(str, number);
                        int fileDesc = open(outputFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                        if (fileDesc < 0)
                        {
                            perror("Couldn't open the file");
                            EXIT_FAILURE;
                        }
                        write(fileDesc, str, number);
                        close(fileDesc);
                    }
                    close(rfd[0]);
                }

                return 1;
            }
        }
        if (isBello)
            return 1;
        pathDirectories = strtok(NULL, ":"); // look for next directory
    }
    free(copy);
    return 0;
}

int reRedirect(char *input, char *command, char **args, char *path, int background, int redirecting, int clearFile, char *outputFile)
{                                      // checks for >>> cases and starts execution
    char *copyOfInput = strdup(input); // Make a copy of the input
    char *firstPart = NULL;
    char *rest = NULL;
    int count = 0;
    for (int i = 0; i < strlen(copyOfInput) - 1; i++)
    {
        if (copyOfInput[i] == '>')
        {
            int j = i;
            while (copyOfInput[j + 1] == '>')
            {
                count++;
                j++;
            }
            if (count == 2)
            {
                divideString(copyOfInput, j, &firstPart, &rest);
                rest += 3;
                parseInput(firstPart, &command, &args, &redirecting, &clearFile, &outputFile);
                executeBuiltInCommands(command, args, path, background, redirecting, clearFile, rest, 1);
                free(firstPart); // Free the memory allocated by divideString
                free(copyOfInput);
                free(command);
                return 1;
            }
            else
            {
                free(copyOfInput);
                return 0;
            }
        }
    }
    free(copyOfInput);
    return 0;
}

void createAlias(char *aliasName, char *aliasCommand)
{ // creates new aliases
    aliases = realloc(aliases, (aliasCount + 1) * sizeof(Alias));
    aliases[aliasCount].aliasName = strdup(aliasName);
    aliases[aliasCount].aliasCommand = strdup(aliasCommand);
    aliasCount++;
}

void freeAliases()
{ // helper function to prevent memory leak
    for (int i = 0; i < aliasCount; i++)
    {
        free(aliases[i].aliasName);
        free(aliases[i].aliasCommand);
    }
    free(aliases);
}

void writeAliases()
{ // file writer to keep aliases
    FILE *file = fopen(ALIASES_PATH, "w");
    if (file != NULL)
    {
        for (int i = 0; i < aliasCount; i++)
        {
            fprintf(file, "%s \"%s\"\n", aliases[i].aliasName, aliases[i].aliasCommand);
        }
        fclose(file);
    }
}

void loadAliases()
{ // reads file and loads aliases
    FILE *file = fopen(ALIASES_PATH, "r");
    if (file == NULL)
        return;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, file)) != -1)
    {
        aliases = realloc(aliases, (aliasCount + 1) * sizeof(Alias));
        char *name = strtok(line, " ");
        trim(name);
        aliases[aliasCount].aliasName = strdup(name);
        char *command = strtok(NULL, "\n");
        trim(command);
        removeQuote(command);
        aliases[aliasCount].aliasCommand = strdup(command);
        aliasCount++;
    }
    free(line);
    fclose(file);
}

int findAlias(char *name, char **command)
{ // checks if the current command is an alias
    // returns 1 if it is, otherwise returns 0
    for (int i = 0; i < aliasCount; i++)
    {
        if (strcmp(aliases[i].aliasName, name) == 0)
        {
            *command = aliases[i].aliasCommand;
            return 1;
        }
    }
    return 0;
}

void bello()
{
    struct passwd *pw = getpwuid(getuid()); // username
    char host[MAX_INPUT_LENGTH];
    gethostname(host, sizeof(host));   // hostname
    char *tty = ttyname(STDIN_FILENO); // tty
    char *shellName = getenv("SHELL"); // shell
    char *home = pw->pw_dir;           // home
    time_t t;
    time(&t); // time
    printf("1. Username: %s\n", pw->pw_name);
    printf("2. Hostname: %s\n", host);
    printf("3. Last Executed Command: %s\n", lastExecutedCommand);
    printf("4. TTY: %s\n", tty);
    printf("5. Current Shell Name: %s\n", shellName);
    printf("6. Home Location: %s\n", home);
    printf("7. Current Time and Date: %s", ctime(&t));
    printf("8. Current number of processes being executed: %d\n", numberOfExecutedProcesses);
}
