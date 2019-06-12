/*
* shelldon interface program

KUSIS ID: 54268 PARTNER NAME: Pelin Durak
KUSIS ID: 27624 PARTNER NAME: Murat Doğan

*/


#include "util.h"
#define MAX_LINE       80 /* 80 chars per line, per command, should be enough. */

void executeCommand(char *args[],pid_t child, int isBackground, bool hasRedirection, bool willAppend);
char *parseCommand(char inputBuffer[], char *args[],int *background, bool *hasRedirection, bool *willAppend);
void oldestchild(char *args[]);

pid_t child;                                /* process id of the child process */
char commandHistory[200][MAX_LINE];         /* holds last 200 commands used */
char *args[MAX_LINE/2 + 1];                 /* command line (of 80) has max of 40 arguments */
char *lastCommand;                          /* holds the last command entered */
char inputBuffer[MAX_LINE];                 /* buffer to hold the command entered */
int status;                                 /* result from execv system call */
bool hasRedirection = false;                /* flag for '>' */
bool willAppend = false;                    /* flag for '>>' */
int historyAccess = 0;                      /* flag for '!!' and '!n' */
int background;                             /* equals 1 if a command is followed by '&' */
int historyIndex = 0;                       /* holds the number of commands entered */
int historyLowerIndex = 1;
int shouldrun = 1;
int kernel_module_param=-1;

int main(void){

  while (shouldrun){            		/* Program terminates normally inside setup */
    background = 0;
    hasRedirection = false;
    willAppend = false;
    historyAccess = 0;

    lastCommand = parseCommand(inputBuffer,args,&background,&hasRedirection,&willAppend);

    //logic for history access
    if (args[0][0]=='!'){
      historyAccess = 1;
      if (strncmp(args[0], "!!", 2) == 0){
        if (strncmp(commandHistory[0], "", 2) == 0){
          printf("No commands in the history. \n");
        }else{
          //clear input buffer
          memset(inputBuffer, 0, sizeof inputBuffer);
          //write last command in history to input buffer
          strcpy(inputBuffer,commandHistory[historyIndex-1]);
          //parse the last command(overrides the parseCommand above)
          lastCommand = parseCommand(inputBuffer,args,&background,&hasRedirection,&willAppend);
        }

      }else{
        int n = atoi(&args[0][1]);
        if(n>historyIndex||n<historyLowerIndex){
          printf("No such command in history. \n");
        }else{
          memset(inputBuffer, 0, sizeof inputBuffer);
          strcpy(inputBuffer,commandHistory[n-1]);
          lastCommand = parseCommand(inputBuffer,args,&background,&hasRedirection,&willAppend);
        }

      }
    }

    //Update history if the command is not 'history, '!!' or '!n'.
    if (strcmp(args[0],"history") != 0 && strcmp(args[0],"!!") != 0 && args[0][0] != '!'){
      strcpy(commandHistory[historyIndex],lastCommand);
      historyIndex++;
      if(historyIndex>10)historyLowerIndex++;
    }

    if (strncmp(inputBuffer, "exit", 4) == 0){
      shouldrun = 0;     /* Exiting from shelldon*/

      //unloads kernel module
      system("sudo rmmod simple");
    }

    if (shouldrun) {
      if (strncmp(args[0], "cd", 2) == 0){//patch for the cd command
        chdir(args[1]);
      }else{
        executeCommand(args,child,background,hasRedirection,willAppend);
      }

    }
  }
  return 0;
}


void executeCommand(char *args[],pid_t child,int isBackground,bool hasRedirection, bool willAppend){

  child = fork();

  if(child==0){
    //inside child process
    if(isBackground)printf("Background process with pid: %d \n",getpid());


    if(hasRedirection){
      int lastParamIndex = getLastNonNullIndex(args);
      //printf("last non null index: %d \n",lastParamIndex);


      if((args[lastParamIndex][0]=='-'||lastParamIndex==0)){
        printf("Syntax error: no file name specified. \n");
      }else{

        char *filename;
        if(args[2]==NULL&& args[3]!=NULL)args[3]=NULL; //bugfix

        //Set filename
        int argslength=0;
        while(args[argslength]!=NULL)argslength++;
        filename=args[argslength-1]; //save file name for open()
        args[argslength-1] = NULL;


        //redirect stdout to the specified file name, call excvp with modified args
        int outputfile;
        //if user enters '>>', the output will be appended to the existing file, else the file will be truncated.
        if(willAppend){
          outputfile=open(filename, O_WRONLY | O_APPEND| O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        }else{
          outputfile=open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        }

        dup2(outputfile,1);//redirect to stdout
        dup2(outputfile, 2);//redirect to stderr


        //executes command with redirection(when user types >/>>)

        if (strncmp(args[0], "history", 7) == 0){
          printHistory(commandHistory);
        }else if(strncmp(args[0], "birdakika", 9) == 0){
          birdakika(args);
        }else if(strncmp(args[0], "google", 6) == 0){
          system(google(args));
        }else if(strncmp(args[0], "codesearch", 10) == 0){
          codesearch(args);
        }else if(strncmp(args[0], "oldestchild", 11) == 0){
          oldestchild(args);
        }else{
          //Execute other commands
          execv(execvHelper(args[0]),args);
        }

        close(outputfile);

      }

    }else{
      //execute without redirection

      if (strncmp(args[0], "history", 7) == 0){
        printHistory(commandHistory);
      }else if(strncmp(args[0], "birdakika", 9) == 0){
        birdakika(args);
      }else if(strncmp(args[0], "google", 6) == 0){
        system(google(args));
      }else if(strncmp(args[0], "codesearch", 10) == 0){
        codesearch(args);
      }else if(strncmp(args[0], "oldestchild", 11) == 0){
        oldestchild(args);
      }else{
        //Execute other commands
        execv(execvHelper(args[0]),args);
      }

    }
    exit(0);

  } else if(child >0){
    //inside parent process
    if(strncmp(args[0], "oldestchild", 11) == 0){
      //update kernel module param with the last entered.
      if(args[1]!=NULL)kernel_module_param = atoi(args[1]);

    }
    if(!isBackground){
      // wait for foreground processes
      wait(NULL);
    }else{
      //do not wait for background processes!
    }
  }



}


void oldestchild(char *args[]){
  //Get args' size
  int argsSize = 0;
  for(int i=0;args[i]!=NULL;i++){
    if(args[i]!=NULL)argsSize++;
  }

  if(argsSize!=2||atoi(args[1])<1){
    printf("invalid pid or argument number. try again. \n");
  }else{
    //load module
    char sCall[300] = {0};
    char buf[16];

    FILE *fd = popen("lsmod | grep simple", "r");

    //check if the module is already loaded
    if (fread (buf, 1, sizeof (buf), fd) > 0){

      //new or same pid, unload and load
      if(atoi(args[1])==kernel_module_param){
        printf ("module is already loaded\n");
      }else{
        printf ("loading module... \n");
        system("sudo rmmod simple");
        sprintf(sCall, "sudo insmod simple.ko p=%s", args[1]);
        system(sCall);
      }

    }else{
      //loading for the first time
      printf ("loading module for the first time...\n");
      sprintf(sCall, "sudo insmod simple.ko p=%s", args[1]);
      system(sCall);

    }

  }
}

char *parseCommand(char inputBuffer[], char *args[],int *background,bool *hasRedirection,bool *willAppend){
  char *lastCmd=malloc(500); //unparsed last command as char*
  int length;                //length of the buffer
  int start=-1;
  int ct=0 ;

  if(!historyAccess){
    //Read user input
    do {
      printf("shelldon> ");
      fflush(stdout);
      length = read(STDIN_FILENO,inputBuffer,MAX_LINE);
    }
    while (inputBuffer[0] == '\n'); /* swallow newline characters */
    if (length == 0)
    exit(0);
  }else{
    //Parse given buffer
    length=MAX_LINE;
  }



  if ( (length < 0) && (errno != EINTR) ) {
    perror("error reading the command");
    exit(-1);           /* terminate with error code of -1 */
  }

  /**
  * Parse the contents of inputBuffer
  */

  for (int i=0;i<length;i++) {
    /* examine every character in the inputBuffer */

    switch (inputBuffer[i]){
      case ' ':
      case '\t' :               /* argument separators */
      appendCharToString(lastCmd,inputBuffer[i]);

      if(start != -1){
        args[ct] = &inputBuffer[start];    /* set up pointer */
        ct++;
      }
      inputBuffer[i] = '\0'; /* add a null char; make a C string */
      start = -1;
      break;

      case '\n':                 /* should be the final char examined */
      appendCharToString(lastCmd,inputBuffer[i]);

      if (start != -1){
        args[ct] = &inputBuffer[start];
        ct++;
      }
      inputBuffer[i] = '\0';
      args[ct] = NULL; /* no more arguments to this command */
      break;

      case '>': /*check for redirection (> or >>) */
      appendCharToString(lastCmd,inputBuffer[i]);

      if( inputBuffer[i+1] == '>'){
        *willAppend=true;
      }
      *hasRedirection = true;

      break;
      default :             /* some other character */
      appendCharToString(lastCmd,inputBuffer[i]);

      if (start == -1)
      start = i;

      if (inputBuffer[i] == '&') {
        *background  = 1;
        inputBuffer[i-1] = '\0';
      }
    } /* end of switch */
  }    /* end of for */

  if (*background)
  args[--ct] = NULL;

  args[ct] = NULL; /* just in case the input line was > 80 */


  return lastCmd;

} /* end of parseCommand routine */
