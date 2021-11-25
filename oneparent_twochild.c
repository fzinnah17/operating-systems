//
//  zinnah_farnazOS1.c
//  zinnah_farnazOS1
//
//  Created by Farnaz Zinnah on 2/27/21.
//  Copyright © 2021 Farnaz Zinnah. All rights reserved.
//

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(const int argc, char * argv[]){
  pid_t pids[2];  //process identifiers
  int pfd[2];     //pipe descriptors

  char * cmd1[] = {"ls", "-F", NULL};
  char * cmd2[] = {"nl", NULL};

  //create the pipe to communicate
  if(pipe(pfd) == -1){
    perror("pipe");
    return EXIT_FAILURE;
  }

  //create the first process
  pids[0] = fork();
  switch(pids[0]){
    case -1:
      perror("fork");
      return EXIT_FAILURE;
    case 0: //the child

      //copy the command output to pipe input
      if(dup2(pfd[1], fileno(stdout)) != fileno(stdout)){
        perror("dup2");
        exit(EXIT_FAILURE);
      }

      //execute the command
      execvp(cmd1[0], cmd1);
      //we get here only on error in execvp()
      perror("execvp");
      exit(EXIT_FAILURE); //exit with error
      break;

    default: //the parent
      close(pfd[1]);  //close pipe input, since the child has it
      break;
  }

  //create the second process
  pids[1] = fork();
  switch(pids[1]){
    case -1:
      perror("fork");
      return EXIT_FAILURE;
    case 0: //the child

      //copy the command input to pipe ouput
      if(dup2(pfd[0], fileno(stdin)) != fileno(stdin)){
        perror("dup2");
        exit(EXIT_FAILURE);
      }

      //execute the command
      execvp(cmd2[0], cmd2);
      //we get here only on error in execvp()
      perror("execvp");
      exit(EXIT_FAILURE); //exit with error
      break;

    default: //the parent
      close(pfd[0]);  //close pipe output, since the child has it
      break;
  }

  //wait for both processes to finish
  waitpid(pids[0], NULL, 0);
  waitpid(pids[1], NULL, 0);

  return EXIT_SUCCESS;
}
