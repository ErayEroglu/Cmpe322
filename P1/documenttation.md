# INTRODUCTION
This program serves as a fundamental custom shell, introducing distinct functionalities and operations compared to default shells on Linux-based OS, particularly suitable for UNIX environments. It can execute programs within the system's PATH variables and also provides built-in functions and operations to the user. Compilation is very simple; a user-friendly makefile is provided. The makefile creates the shell program, which is an executable file named "myshell".

# IMPLEMENTATION

The shell program operates in a fork-exec manner. It searches for executables based on the PATH variable order. Error handling is emphasized, and the shell must not terminate with an error, instead providing clear error messages. In cases where a command is not found in the PATH variables or among built-in commands, the shell prints a console message: “Command is not found”.

The shell has various features: it supports background processing (&), redirection operators (">" and ">>"), alias creation, and a unique re-redirection operator (">>>") which reverses the output of a program and writes it to a file. It also terminates with the “exit” command. 

The shell also has another built-in command, “bello,” which displays eight information items about the user, such as username, hostname, last executed command, TTY, current shell name, home location, current time and date, and the number of currently executed processes. It is a very useful method to get information about the system.

The program has specific restrictions, limiting the maximum input length to 128 characters.  While longer commands are technically feasible, it is considered unusual. Consequently, as a design choice, 128 characters is decided as the upper limit. 

Memory allocation is mostly dynamic; it is preferred because it provides a more flexible approach while handling different types of commands and their arguments. These allocations are freed accordingly after their utilization, which ensures memory efficiency.

# CONCLUSION
In summary, the program mainly focuses on creating a customized shell with specific functionalities, offering a unique implementation on macOS or Ubuntu. It is not as sophisticated as bash or zsh, however, it provides great functionalities to the user and it is very easy to use.  


