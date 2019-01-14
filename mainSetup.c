#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#define CREATE_FLAGS (O_WRONLY | O_TRUNC | O_CREAT )
#define CREATE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define CREATE_APPENDFLAGS (O_WRONLY | O_APPEND | O_CREAT )
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */

/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */
		
void setup(char inputBuffer[], char *args[],int *background, int *IO_Redirection)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct,
        ioCounter;     /* index of where to place the next parameter into args[] */

    ct = 0;
    ioCounter=0;
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	exit(-1);           /* terminate with error code of -1 */
    }

    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
	    case ' ':
	    case '\t' :               /* argument separators */
		if(start != -1)
    {
        args[ct] = &inputBuffer[start];    /* set up pointer */
        ct++;
		}
    inputBuffer[i] = '\0'; /* add a null char; make a C string */
		start = -1;
		break;

            case '\n':                 /* should be the final char examined */
		if (start != -1)
    {
        args[ct] = &inputBuffer[start];
		    ct++;
		}
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
		break;

	    default :             /* some other character */
		if (start == -1)
		    start = i;

    if (inputBuffer[i] == '&'){
		*background  = 1;
        	inputBuffer[i-1] = '\0';
		}
    if(inputBuffer[i] == '>' || inputBuffer[i] == '<' ){
       if(inputBuffer[i]==inputBuffer[i+1]){
         *IO_Redirection +=5; //if there is redirection >> count until 5 of them
       }
       else{
         *IO_Redirection += 1; //control there is IO_Redirection or not. If there is, count how many are
       }
     }


	} /* end of switch */
     }    /* end of for */
     args[ct] = NULL; /* just in case the input line was > 80 */

	for (i = 0; i <= ct; i++){
 	   printf("",i,args[i]);
 }
      Command_Direct( *background, args,  ct, *IO_Redirection );


} /* end of setup routine */

//IN THIS FUNCTION THE BUILD IN COMMANDS AND REDIRECTIONS ARE CREATED

void Command_Direct(int background, char *args[], int counter, int IO_Redirection  )
{
  	pid_t pid;
  	DIR *directory;
  	const char* env_var;
  	char OP;
  	char inputBuffer[MAX_LINE];
 	int shutdown;
/* PART B : BUILT IN COMMANDS */
// using strcmp to compare input string with our commands if same execute command
  	if(strcmp(args[0],"alias")==0){
		printf("");
	}
  	else if(strcmp(args[0],"unalias")==0){
		printf("a;");
	}
	//Used system clear to get the clear command
  	else if(strcmp(args[0],"clr")==0){
		system("clear");
	}	
	//When we type the exit command shutdown shell
 	else if(strcmp(args[0],"exit")==0){
		exit(0);

	}
// PART C : I/O REDIRECTION 
//STANDARD OUTPUT '>'
	// Check if the Source file exist or not
	//Also fix our > to tell that it is Output redirection
  	else if((IO_Redirection==1) && (*args[counter-2]=='>')){ 
  		char* path="/bin/"; //Start Path
    		pid= fork();
	// Create file if needed 
	// Create flag and mode are used from C librarys to open and create files
    	if(pid==0){
       		int kg;
       		kg = open(args[counter-1],CREATE_FLAGS,CREATE_MODE); 
	// Check if file will open
       if(kg == -1){
          perror("Failed to open file");
          return 1;
       }
	//Check if redirection is finished
       if(dup2(kg,STDOUT_FILENO) == -1){
          perror("Failed to redirect standart output");
          return 1;
      }
	//Check if the file is gonna close
      if(close(kg) == -1) {
          perror("Failed to close the file");
          return 1;
      }

      char kp[50];
	//create a path for the redirection
	//copy path
      strcpy(kp, path);
	//add args for different commands like ls or cat or sort ...
      strcat(kp, args[0]); 

      args[counter-1] = NULL;
      args[counter-2] = NULL; //The last two arrays will be NULL
	//Print the errors in stderr like requested
      if (execv(kp,args) == -1){
        fprintf(stderr, "Child process couldn't run \n");
      }
}
     else if(pid>0){
       wait(pid,NULL,0);
     }
  }
//STANDARD INPUT '<'
//Basically the same Idea as output just > is < now and also we O_RDWR to write to a new file we dont need output in cmd also is not stdout now is stdin others same
     else if((IO_Redirection==1) && (*args[counter-2]=='<')) {
    	char* path="/bin/"; 
    	pid= fork();
       if(pid==0){
       int kg;
	//open existing file in the directory
       kg = open(args[counter-1],O_RDWR); 
       printf("kg = %d\n",kg );

       if(kg == -1){
          perror("Failed to open file");
          return 1;
       }

       if(dup2(kg,STDIN_FILENO) == -1){
          perror("Failed to redirect standart input");
          return 1;
      }

      if(close(kg) == -1){
          perror("Failed to close the file");
          return 1;
      }
      char kp[50];

      strcpy(kp, path); 
      strcat(kp, args[0]);
    
      args[counter-1] = NULL;
      args[counter-2] = NULL; 
      if (execv(kp,args) == -1){
        fprintf(stderr, "Child does not work \n");
      }
     }
     else if(pid>0){
       wait(pid,NULL,0);
     }
  }
// STANDARD OUTPUT APPEND '>>'
//Again the same logic as the last two just now we use a different type of condition
//in the internet I saw that for >> they have a condition to be equal to 6
//others parts are continued as in the other ones 
  else if((IO_Redirection==6)) {	
    char* path="/bin/"; 
    pid= fork();
    if(pid==0){
       int kg;
	// create new folder
       kg = open(args[counter-1],CREATE_APPENDFLAGS,CREATE_MODE); 
       if(kg == -1){
          perror("Failed to open file");
          return 1;
       }
       if(dup2(kg,STDOUT_FILENO) == -1){
          perror("Failed to redirect standart output");
          return 1;
      }

      if(close(kg) == -1){
          perror("Failed to close the file");
          return 1;
      }

      char kp[50];

      strcpy(kp, path); 
      strcat(kp, args[0]);
    
      args[counter-1] = NULL;
      args[counter-2] = NULL; 

      if (execv(kp,args) == -1){
        fprintf(stderr, "Child does not work \n");
      }

     }
     else if(pid>0){
       wait(pid,NULL,0);
     }
  }
//Read INPut and put in ouput '<' and '>'
//If we have two redirections like < and > then enter this condition
//and then it must be smaller than 7 to work and the now 
//we just dont use nor input files output
//now we include both of them
  else if(IO_Redirection>=2) { 
 
    if(IO_Redirection < 7){ 
      pid = fork();
      if(pid == 0){
        if(pid == -1){
            fprintf(stderr, "Chid process error \n");
            return 0;
        }

        int inputFile;
        int outputFile;
        char* path="/bin/"; 
	//input file open
        inputFile = open(args[counter-3],O_RDWR,CREATE_MODE); 
	//new file
        outputFile = open(args[counter-1],CREATE_FLAGS,CREATE_MODE); 



        if(inputFile == -1){
          perror("Failed to open file");
          return 1;
        }

        if(dup2(inputFile,STDIN_FILENO) == -1){
          perror("Failed to redirect standart output");
          return 1;
        }

        if(close(inputFile) == -1){
          perror("Failed to close the file");
          return 1;
        }

        if(outputFile == -1)
        {
          perror("Failed to open file");
          return 1;
        }
        if(dup2(outputFile,STDOUT_FILENO) == -1){
          perror("Failed to redirect standart output");
          return 1;
        }

        if(close(outputFile) == -1){
          perror("Failed to close the file");
          return 1;
        }


        char kp[50];
//creating path
        strcpy(kp, path); 
        strcat(kp, args[0]);
        args[counter-4] = NULL;

        if (execv(kp,args) == -1){
          fprintf(stderr, "Child does not work \n");
        }
      }
    }
	// if there is A redirection
    else 
    {
      pid = fork();

      if(pid == 0){
        if(pid == -1){
            fprintf(stderr, "Chid process error \n");
            return 0;
        }
        int inputFile;
        int outputFile;
        char* path="/bin/"; 

        inputFile = open(args[counter-3],O_RDWR,CREATE_MODE);
        outputFile = open(args[counter-1],CREATE_APPENDFLAGS,CREATE_MODE);



        if(inputFile == -1){
          perror("Failed to open file");
          return 1;
        }

      	if(dup2(inputFile,STDIN_FILENO) == -1){
          perror("Failed to redirect standart output");
          return 1;
        }

        if(close(inputFile) == -1){
          perror("Failed to close the file");
          return 1;
        }

        if(outputFile == -1){
          perror("Failed to open file");
          return 1;
        }
        if(dup2(outputFile,STDOUT_FILENO) == -1){
          perror("Failed to redirect standart output");
          return 1;
        }

        if(close(outputFile) == -1){
          perror("Failed to close the file");
          return 1;
        }

        char kp[50];

        strcpy(kp, path); 
        strcat(kp, args[0]);
        args[counter-4] = NULL;

        if (execv(kp,args) == -1)
{
          fprintf(stderr, "Child does not work \n");
        }
      }
    }


  }
//PART A: FOREGROUND and BACKGROUND
// This first part is for the foreground processes
  if(background==0 && IO_Redirection==0) 
  {
    pid=fork();
    if(pid<0)
    {
      printf(stderr,"child process cannot be created");
    }
	//parent process
    else if (pid>0) 
    {
      wait(&pid); //parent process waits child process until it finishes
    }
    else if(pid==0) //child process
    {
        printf(stderr,"\nforeground process running\n");
        if ((directory = opendir(args[counter-1])) != NULL) //If there is a directory
        {
          OP = args[0];
          int index=0;

          while (args[index]!=NULL)
          {
              args[index]=args[index+1]; //shift to the left
              index++;
          }
          execv("/bin/"+OP,args); //execute program
        }
        else
        {
          execvp(args[0],args);
        }

    }

  }
//If the processes are background processes '&'
  else if(background==1 && IO_Redirection==0)
  {
      pid=fork();
      shutdown++;
	// If the process is a child
    if (pid == 0) 
     {
       fprintf(stderr,"\nbackground process running\n");
 	//Check If there is a directory
       if ((directory = opendir(args[counter-1])) != NULL)
       {
         OP = args[0];
         int index=0;

         while (args[index]!=NULL)
         {
           args[index]=args[index+1]; //shifting array to the left
           index++;
         }
         execv("/bin/"+OP,args);
       }
       else
       {
         execvp(args[0],args);
       }

     }
      shutdown--;
  }

  IO_Redirection=0;
  background = 0; // default value of background
  setup(inputBuffer, args, &background,&IO_Redirection);


}


char* getPath(char fileName[]);
int pathSearch(char *pathP, char* fileNameP);

//  searches given file name in given path and returns 1 for found, 0 for not found
int pathSearch(char *path,char* fileName) {
  DIR *d;
  struct dirent *dir;
	int found = 0;
  d = opendir(path);
    if (d) {
      while ((dir = readdir(d)) != NULL) {
		    if(strcmp(dir->d_name,fileName) == 0)  {
          found = 1; // match found
			    break;
		    }
      }
      closedir(d);
    }
    return(found);
}


// gets filename and colloborates with pathSearc function to return path location
char* getPath (char fileName[]) {
  char *pathvar, *pch;
  char* foundPath = (char*)malloc(100); // allocate memory to hold path variable
  int found = 0;
  pathvar = getenv("PATH");   // get path enviroment variable
  pch = strtok (pathvar,":"); // delimiter for path variable
  while (pch != NULL) {   // loop until a path is found
    found = pathSearch(pch,fileName); // return 1 for found
    if(found) {   // copy to foundPath when found
    	strcpy(foundPath,pch);
      break;
    }
    pch = strtok (NULL, ":");
  }
  if (strcmp(foundPath,"") == 0)    // return 'e' for error handling when not found
	  return "e";
  else
	  return foundPath;
}

int main(void)
{
            char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
            int background; /* equals 1 if a command is followed by '&' */
            char *args[MAX_LINE/2 + 1]; /*command line arguments */
            int IO_Redirection;
            while (1){
                        background = 0;
                        printf("myshell: ");
                        /*setup() calls exit() when Control-D is entered */
			fflush(stdout);                        
			IO_Redirection=0;
			
                        setup(inputBuffer, args, &background, &IO_Redirection);

                        /** the steps are:
                        (1) fork a child process using fork()
                        (2) the child process will invoke execv()
						(3) if background == 0, the parent will wait,
                        otherwise it will invoke the setup() function again. */
            }
}

