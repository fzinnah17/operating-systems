# Operating Systems Course Projects

## Project 1: oneparent_twochild.c

### About
This C program demonstrates inter-process communication and pipe handling in a UNIX environment. It creates two child processes from one parent process, using `fork()` and `pipe()`. The first child executes the `ls` command and the second executes the `nl` command, with the output of `ls` being piped as input to `nl`.

### Technical Highlights
- Utilizes `fork()` for process creation.
- Implements `pipe()` for inter-process communication.
- Employs `dup2()` for file descriptor duplication.
- Executes external commands (`ls` and `nl`) using `execvp()`.

## Project 2: shell.c

### About
This custom shell program in C offers various UNIX shell functionalities, including clearing the screen, listing directory contents, and handling directories and files. It showcases the usage of system calls and process management in a shell environment.

### Key Features
- Implements a basic shell interface.
- Executes commands like `clear`, `mkdir`, and `chdir`.
- Custom `cmd_tree()` and `cmd_list()` functions for directory manipulation and file listing.
- Utilizes `stat()` for file information retrieval, replicating `ls -l` functionality.

Each project encapsulates key concepts of operating systems, demonstrating process management, file handling, and system call usage, essential for understanding the internal workings of an OS.
