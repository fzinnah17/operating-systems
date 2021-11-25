#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/dir.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

//Run the clear command in external process
static int clear_screen(){

  //create the child process
  const pid_t pid = fork();
  if(pid == -1){  //if fork failed
    perror("fork");
    return -1;  //return error code

  }else if(pid == 0){ //this is the child

    //run the clear command
    execlp("clear", "clear", NULL);
    //if execl fails, we get here
    perror("execl"); //report why exec failed
    exit(EXIT_FAILURE); //exit the child process
  }else{
    int status;
    if(wait(&status) == -1){
      perror("wait");
    }
  }

  return 0;
}

//Function for shell command tree
static int cmd_tree(){

  int i;
  char buf[10]; //filename buffer

  //Create the directory
  if(mkdir("Dir0", S_IRWXU) == -1){
    perror("mkdir");
    return -1;
  }

  //change the directory
  if(chdir("Dir0") == -1){  //if chdir failed
    perror("chdir");  //show why chdir failed
    return -1;  //return error
  }

  //create 3 empty files
  for(i=1; i < 4; i++){

    //create the filename
    snprintf(buf, sizeof(buf), "t%d.txt", i);

    //create the empty file. Will fail if file exists!
    const int fd = open(buf, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
    if(fd == -1){ //if open failed
      perror("open");
      return -1;  //return error for tree function
    }
    close(fd);
  }

  //Create the second directory
  if(mkdir("Dir1", S_IRWXU) == -1){
    perror("mkdir");
    return -1;
  }

  return 0;
}

//Show info for a file, in one line, like ls
static void print_stat(const char * fname, FILE * output){

	struct passwd pwent, *pwentp;
  struct group grp, *grpt;
  struct tm time;
  char buf[256];
  char filetype = '?';

  //get file info
  struct stat st;
  if (stat(fname, &st) == -1){
    perror("stat");
    return;
  }

  //first character is the file type
	     if (S_ISSOCK(st.st_mode)) filetype = 's';
	else if (S_ISLNK(st.st_mode))  filetype = 'l';
	else if (S_ISREG(st.st_mode))  filetype = '-';
	else if (S_ISBLK(st.st_mode))  filetype = 'b';
	else if (S_ISDIR(st.st_mode))  filetype = 'd';
	else if (S_ISCHR(st.st_mode))  filetype = 'c';
	else if (S_ISFIFO(st.st_mode)) filetype = '|';

  //print the permissions
	fprintf(output, "%c%c%c%c%c%c%c%c%c%c.", filetype,
			st.st_mode & S_IRUSR ? 'r' : '-', st.st_mode & S_IWUSR ? 'w' : '-', st.st_mode & S_IXUSR ? 'x' : '-',
			st.st_mode & S_IRGRP ? 'r' : '-', st.st_mode & S_IWGRP ? 'w' : '-',	st.st_mode & S_IXGRP ? 'x' : '-',
			st.st_mode & S_IROTH ? 'r' : '-', st.st_mode & S_IWOTH ? 'w' : '-',	st.st_mode & S_IXOTH ? 'x' : '-');

  //show how many links we have
	fprintf(output, " %i", (int)st.st_nlink);

  //show user
	if (getpwuid_r(st.st_uid, &pwent, buf, sizeof(buf), &pwentp) == 0)
		fprintf(output, " %-10s", pwent.pw_name);
	else
		fprintf(output, " %-10i", st.st_uid);

  //show group
	if (getgrgid_r (st.st_gid, &grp, buf, sizeof(buf), &grpt) == 0)
		fprintf(output, " %-10s", grp.gr_name);
	else
		fprintf(output, " %-10i", st.st_gid);

  //show size
	fprintf(output, " %10i", (int)st.st_size);

  //show modification time
	localtime_r(&st.st_mtime, &time);
	strftime(buf, sizeof(buf), "%d %h %H,%M", &time);	/* Get localized date string. */

  //show file name
  fprintf(output, " %s %s\n", buf, fname);
};

static int list_directory(FILE * fout){
  unsigned int total_blocks = 0;
	struct dirent *dent;
	DIR *dir;

  //open the current directory to count blocks, which are printed on first line
	if (!(dir = opendir("."))) {
		perror("opendir");
		return -1;
	}

  //for each directory entry
	while ((dent = readdir(dir))) {
    struct stat st;

		//skip the current and previous directory links
		if (!strcmp(dent->d_name, ".") ||
        !strcmp(dent->d_name, "..")){
			continue;
    }

		if (stat(dent->d_name, &st) == -1){
			continue;  //move to next
    }

    int blocks = st.st_size / 4096;
    if(st.st_size % 4096){
      blocks++;
    }
    total_blocks += blocks;
	}
	closedir(dir);

  //printf("total %d\n", total_blocks);
  fprintf(fout, "total %d\n", total_blocks);

  //open the current directory
	if (!(dir = opendir("."))) {
		perror("opendir");
		return -1;
	}

	while ((dent = readdir(dir))) {  //while we have directory entries

		//skip the . and ..
		if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")){
			continue;
    }

    //show output like "ls -l" for the filename
    //print_stat(dent->d_name, stdout);
    print_stat(dent->d_name, fout);
	}
	closedir(dir);

  return 0;
}


//Function for shell command tree
static int cmd_list(const int clear){

  if(clear){
    //magic number to clear Unix/Linux terminal
    //printf("\x1b\x5b\x48\x1b\x5b\x32\x4a\x1b\x5b\x33\x4a");
    //printf("\x1B[2J");
    //system("clear");
    clear_screen();
  }

  FILE * fout = fopen("t1.txt", "w");
  if(fout == NULL){
    perror("fopen");
    return -1;
  }

  list_directory(fout);
  list_directory(stdout);

  fclose(fout);

  //rename the file from t1.txt to tree.txt
  if(rename("t1.txt", "tree.txt") == -1){
    perror("rename");
    return -1;
  }

  return 0;
}

//Print a file to another file. Can be stdout, since its a file too
static int cat_file(const char * filename, FILE * fout){
  char buf[100];  //buffer for reading and writing to fout
  size_t bytes; //how many bytes we read from input file

  //open the source file
  FILE * fin = fopen(filename, "r");
  if(fin == NULL){
    perror("fopen");
    return -1;
  }

  //read from input file
  while(( bytes = fread(buf, 1, sizeof(buf), fin)) > 0){
    //write to output file
    if(fwrite(buf, 1, bytes, fout) != bytes){
      break;
    }
  }
  fclose(fin);

  return 0;
}

static int cmd_path(){
  char buf[1000];

  //get the name of current directory
  if(getcwd(buf, sizeof(buf)) == NULL){
    perror("getcwd");
    return -1;
  }

  //print current directory on screen
  printf("%s\n", buf);

  //print to t2.txt
  FILE * fout = fopen("t2.txt", "w");
  if(fout == NULL){
    perror("fopen");
    return -1;
  }
  fprintf(fout, "%s\n", buf);
  fclose(fout);

  //rename it to path.txt
  if(rename("t2.txt", "path.txt") == -1){
    perror("rename");
    return -1;
  }

  //open the new file, that we will concatenate too
  fout = fopen("t3.txt", "w");
  if(fout == NULL){
    perror("fopen");
    return -1;
  }

  //concatenate the tree.txt and path.txt to t3.txt
  cat_file("tree.txt", fout);
  cat_file("path.txt", fout);

  fclose(fout);

  if(rename("t3.txt", "log.txt") == -1){
    perror("rename");
    return -1;
  }

  //delete tree.txt and path.txt
  if( (unlink("tree.txt") == -1) ||
      (unlink("path.txt") == -1) ){
    perror("unlink");
    return -1;
  }

  return 0;
}

int main(void){
  int i;
  char *list[4];
  int num_cmd = 0;

  char line[1024];  //buffer for user input of line

  //loop until user enters exit
  while(1){

    //show prompt
    printf("shell | ");

    //get the line command from user
    if(fgets(line, sizeof(line), stdin) == NULL){ //if fgets failed
      if(ferror(stdin)){  //and we have an error
        perror("fgets");  //report it
      }
      break;  //stop major loop
    }

    size_t len = strlen(line);
    if(line[len - 1] == '\n'){  //if line ends with newline
      line[len - 1] = '\0'; //replace it with end of string
      len = len - 1;  //reduce lenght of line
    }

    //check what command we have
    if(strcmp(line, "tree") == 0){
      cmd_tree();

    }else if(strcmp(line, "list") == 0){
      const int clear = 1;  //tells commnand to clear screen
      cmd_list(clear);

    }else if(strcmp(line, "path") == 0){
      cmd_path();

    }else if(strcmp(line, "exit") == 0){
      // print a list of last 4 commands
      for(i=0; i < num_cmd; i++){
        printf("%d. %s\n", i, list[i]);
      }

      list_directory(stdout);  //list directory like ls -l, without clearing screen
      getchar();    //wait for user to press return key

      break;  //stop major loop
    }

    if(num_cmd == 4){

      free(list[0]);
      for(i=1; i < 4; i++){
        list[i-1] = list[i];
      }
      num_cmd = 3;
    }
    list[num_cmd++] = strdup(line);
  }

  for(i=0; i < num_cmd; i++){
    free(list[i]);
  }

  return 0;
}
