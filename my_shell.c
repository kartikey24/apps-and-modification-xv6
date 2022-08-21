#include  <stdio.h>
#include  <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/

int flag = 0;
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


int cd(char **args)
{
  if (args[1] == NULL || args[2]!=NULL) {
    printf("Shell: Incorrect command\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("Shell");
    }
  }
  return 1;
}

int start(char **args)
{

  int pid, pid_wait;
  int status;

  pid = fork();
  if (pid == 0) {
  	setpgid(0,9);

    if (execvp(args[0], args) == -1) {
      printf("%s: command not found\n",args[0]);
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("Shell");
  } else {
    do {
      pid_wait = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int check_cd(char **args)
{

	if (args[0] == NULL) {
	return 1;
	}

	if (strcmp(args[0], "cd") == 0) {
		return cd(args);
	}
  

  return start(args);
}

int execute_serial(char **args){
	int i=0;
	int j=0;
	int k=0;
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	while(args[k]!=NULL){
		if(flag==1) return 0;
		if(strcmp(args[k],"&&")==0){
			check_cd(tokens);
			for(i=0;tokens[i]!=NULL;i++){
				free(tokens[i]);
			}
			j=0;
			// printf("debug %d %d\n",i,j);
		}
		else{
			tokens[j] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
			strcpy(tokens[j++], args[k]);
		}
		k++;
	}
	if(k>0 && strcmp(args[k-1],"&&")!=0){

		if(flag==1) return 0;
		if(tokens[0]!=NULL){
			check_cd(tokens);
			j=0;
		}
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
	}
	free(tokens);

	
	return 1;


}

int execute_parallel(char **args){
	int i=0;
	int j=0;
	int k=0;
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));

	int pid, pid_wait;
	int status;

	while(args[k]!=NULL){
		if(strcmp(args[k],"&&&")==0){
			pid = fork();
			if (pid == 0) {
				setpgid(0,9);
				if (execvp(tokens[0], tokens) == -1) {
					printf("%s: command not found\n",args[0]);
				}
				exit(EXIT_FAILURE);
			} else if (pid < 0) {
				perror("Shell");
			}
			for(i=0;tokens[i]!=NULL;i++){
				free(tokens[i]);
			}
			j=0;
			// printf("debug %d %d\n",i,j);
		}
		else{
			tokens[j] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
			strcpy(tokens[j++], args[k]);
		}
		k++;
	}
	if(k>0 && strcmp(args[k-1],"&&")!=0){
		if(tokens[0]!=NULL){
			pid = fork();
			if (pid == 0) {

				if (execvp(tokens[0], tokens) == -1) {
					printf("%s: command not found\n",args[0]);
				}
				exit(EXIT_FAILURE);
			} else if (pid < 0) {
				perror("Shell");
			}
			j=0;
		}
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
	}
	free(tokens);
	if(pid>0){
	do {
      pid_wait = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	
	return 1;


}
int execute_bg(char **args, int &pid)
{

  int pid_wait;
  int status;

	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	int i=0;

	while(strcmp(args[i],"&")!=0){
		tokens[i] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
			strcpy(tokens[i], args[i]);
			i++;
	}



  pid = fork();
  if (pid == 0) {

    if (execvp(tokens[0], tokens) == -1) {
      printf("%s: command not found\n",args[0]);
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("Shell");
  } 
  
	for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
  return 1;
}

int execute(char **args, int mode, int &num){
	if(mode==0) check_cd(args);
	else if(mode==2) execute_serial(args);
	else if(mode==3) execute_parallel(args);
	else if(mode==1) execute_bg(args,num);

	return 1;
}

void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    killpg(9,SIGINT);
	flag=1;
}

int main(int argc, char* argv[]) {


	signal(SIGINT, sigintHandler);
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;

	int pid=0;
	int pid_wait;
	int status;
	int bg=0;
	while(1) {			
		/* BEGIN: TAKING INPUT */
		
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		// printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
   
       //do whatever you want with the commands, here we just print them
	   int mode = 0;
	   int num=0;

	   for(i=0; tokens[i]!=NULL; i++){
		   if(strcmp(tokens[i],"&")==0){
			   mode=1;
				bg++;
		   }
		   if(strcmp(tokens[i],"&&")==0){
			   mode=2;
			   num++;
		   }
		   if(strcmp(tokens[i],"&&&")==0){
			   mode=3;
			   num++;
		   }
	   }
	   if(pid>0){
		   int n=bg;
			for(int i=0; i<n; i++){
				pid_wait = waitpid(pid, &status, WUNTRACED);
				if(WIFEXITED(status)){
					bg--;
					printf("Shell: Background process finished\n");
				}
			}
		}

		if(strcmp(tokens[0],"exit")==0){

			if(pid>0){
			for(int i=0; i<bg; i++){
				pid_wait = waitpid(pid, &status, WUNTRACED);
				if(!WIFEXITED(status)){
					kill(pid_wait, SIGQUIT);
				}
			}
		}
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
			return 0;
		}
		execute(tokens, mode, pid);
		

		// for(i=0;tokens[i]!=NULL;i++){
		// 	printf("found token %s (remove this debug output later)\n", tokens[i]);
		// }
       
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
