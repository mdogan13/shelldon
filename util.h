#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define MAX_LINE 80

int getLastNonNullIndex(char *args[]);
void appendCharToString(char* string, char c) ;
char *execvHelper(char *arg);
void printHistory(char commandHistory[200][MAX_LINE]);
char *google(char *args[]);
void birdakika(char *args[]);
void searchFile(char *word , char *file);
void searchDirectory(char *path,bool isRecursive, char* word);
void codesearch(char *args[]);


int getLastNonNullIndex(char *args[]){
  //helper function for edge cases
  for (int i=0; i<(MAX_LINE/2) + 1; i++) {
    if(args[i]==NULL){
      return i-1;
    }
  }
  return 0;
}

void appendCharToString(char* string, char c) {
  int len = strlen(string);
  string[len] = c;
  string[len+1] = '\0';
}


char *execvHelper(char *arg){
  //glues "/bin/" and any UNIX command.
  char *path="/bin/";
  char *target = malloc(strlen(path) + strlen(arg) + 1);

  strcpy(target,path );
  strcat(target,arg );

  return target;
}

void printHistory(char commandHistory[200][MAX_LINE]){

  if (strncmp(commandHistory[0], "", 2) == 0){
    printf("No commands in the history. \n");
  }else{
    //print last 10 commands from history
    int count = 0;
    for (int i = 99; i >= 0; i--){
      if(!(commandHistory[i][0]=='\0')){
        printf("%d %s",i+1,commandHistory[i]);
        count++;
      }
      if(count==10)break;
    }
  }

}

char *google(char *args[]){
  //Take user's query from args[1], glue it to url and put in a new string called target.
  char *url="open http://www.google.com/search?q="; //xdg-open instead of open in linux!!
  char *query=args[1];
  char *target=malloc(500);

  strcpy(target,url );

  for (int i=0; i<strlen(query); i++) {
    //The user is excpected to use commas to seperate words while writing queries.
    //The commas are replaced with '+' characters so the words can be seperated in google search.
    char a =query[i];
    char plus='+';

    if(strncmp(&a, ",",1) == 0){
      appendCharToString(target, plus);
    }else{
      appendCharToString(target,a);

    }
  }

  return target;
}


void birdakika(char *args[]){
  char sCall[300] = {0}; //system arg
  char *command=malloc(500); //what goes after echo in the system call
  char *mpgpath=malloc(500); //part of command
  char cwd[300];// part of command (path to the audio)
  char *time = args[1]; //time
  char *t1; //hour
  char *t2; //minute
  char *fname = args[2]; //filename

  //grab hour and minute from time
  char *token = strtok(time, ".");
  t1=token;
  token = strtok(NULL, ".");
  t2=token;

  sprintf(mpgpath,"%s %s * * * /usr/local/Cellar/mpg321/0.3.2/bin/mpg321 -n 2300 ",t2,t1);

  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    sprintf(command,"%s%s/%s",mpgpath,cwd,fname);


  }else {
    perror("getcwd() error");
  }
  // printf("cmd: %s \n",command);
  sprintf(sCall, "(crontab -l || true; echo \"%s\")| crontab -", command);
  system(sCall);
}

void searchFile(char *word , char *file){
  char line[1024] ;
  FILE* fp = fopen(file, "r") ;
  int lineNumber =1;
  //read the file
  while (fgets(line , sizeof(line) , fp )!= NULL){
    // print the line if there's an occurence
    if (strstr(line , word )!= NULL){
      printf("%d:  %s -> %s",lineNumber,file,line);
    }
    lineNumber++;
  }
  fclose(fp);
}


void searchDirectory(char *path,bool isRecursive, char* word){
  struct dirent *de;  // pointer for directory entry

  DIR *dr = opendir(path); //returns a pointer of DIR type.

  if (dr == NULL)  printf("Could not open current directory" );

  char new_path[1024];

  while ((de = readdir(dr)) != NULL){

    if(de->d_type==DT_DIR){
      if(isRecursive&&(!(strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0))) {
        // look for subdirectories (by updating the path with the directory found) if codesearch is called with -r
        snprintf(new_path, sizeof(new_path), "%s/%s", path, de->d_name);
        searchDirectory(new_path,isRecursive,word);
      }

    }else{
      //search file for the word
      char filePath[1024];
      sprintf(filePath,"%s/%s", path, de->d_name);
      searchFile(word,filePath);
    }

  }

  closedir(dr);

}


void codesearch(char *args[]){
  /*
  0      1     2    3
  codesearch word                //just current directory
  codesearch word  -f   path     //targeted
  codesearch -r   word           //recursive

  */

  //Get args' size
  int argsSize = 0;
  for(int i=0;args[i]!=NULL;i++){
    if(args[i]!=NULL)argsSize++;
  }

  //depending on number of arguments, call searchDirectory or searchFile with proper arguments.
  if(argsSize==2){
    searchDirectory(".",false,args[1]);
    printf(" \n");
  }else if (argsSize ==3 && (strncmp(args[1],"-r",2)==0)){

    searchDirectory(".",true,args[2]);
    printf(" \n");
  }else if (argsSize == 4 && (strncmp(args[2],"-f",2)==0) ){
    searchFile(args[1],args[3]);
    printf(" \n");
  }else{
    printf("illegal usage. try \n codesearch [word] \n codesearch [word] [-f] [path] \n codesearch [-r] [word] \n");
  }



}



#endif
