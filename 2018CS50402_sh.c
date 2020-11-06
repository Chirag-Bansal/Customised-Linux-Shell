#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

#define DELIM " \t\r\n\a"

int execute(char** args);
char* read_line(void);
char** split_line(char* line);

char *replaceWord(const char *s, const char *oldW,const char *newW) 
{ 
    char *result; 
    int i, cnt = 0; 
    int newWlen = strlen(newW); 
    int oldWlen = strlen(oldW); 
    for (i = 0; s[i] != '\0'; i++) 
    { 
        if (strstr(&s[i], oldW) == &s[i]) 
        { 
            cnt++; 
            i += oldWlen - 1; 
        } 
    } 
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1); 
    i = 0; 
    while (*s) 
    { 
        if (strstr(s, oldW) == s) 
        { 
            strcpy(&result[i], newW); 
            i += newWlen; 
            s += oldWlen; 
        } 
        else
            result[i++] = *s++; 
    } 
  
    result[i] = '\0'; 
    return result; 
} 

int main(int argc, char* argv[])
{
	char *line;
	char **args;
	char* old = "|";
	char* new = " | ";
	char* old1 = "<";
	char* new1 = " < ";
	char* old2 = ">";
	char* new2 = " > ";
	int status = 1;
	printf("%s\n", "This is Chirag Bansal's Shell");
	while(status) {
		printf("shell>");
    	line = read_line();
    	line = replaceWord(line,old,new);
    	line = replaceWord(line,old1,new1);
    	line = replaceWord(line,old2,new2);
    	args = split_line(line);
    	status = execute(args);
    	free(line);
    	free(args);
	}
	return 0;
}

char* read_line(void){
	char *line = NULL;
	ssize_t bufsize = 0;
	getline(&line, &bufsize, stdin);
	return line;
}

char **split_line(char *line)
{
  int bufsize = 64, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    printf("Allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += 64;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        printf("Allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int pwd(char **args){
	char cwd[1000];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("Current working dir: %s\n", cwd);
   } else {
       perror("getcwd() error");
       printf("getcwd() error");
       return 1;
   }
   return 1;
}

int cd(char** args)
{
  if (args[1] == NULL) {
    printf("Need a directory\n");
  } 
  else {
  	int i = 2;
	while(args[i] != NULL){
		strcat(args[1],args[i]);
		i++;
	}
	if(strcmp(args[1],"..") == 0)
		printf("%s\n", "Going into parent directory");
	else
		printf("%s\t%s\n", "Going into directory : ", args[1]);
  	chdir(args[1]);
  }
  return 1;
}

int shell_mkdir(char **args){
	if(args[1] == NULL){
		printf("Need a directory name\n");
	}
	else{
		int i = 1;
		while(args[i] != NULL){
			mkdir(args[i],0777);
			printf("%s\t%s\n", "Created file : ", args[i]);
			i++;
		}
	}
	return 1;
}

int shell_rmdir(char **args){
	if(args[1] == NULL){
		printf("Need a directory \n");
	}
	else{
	  	int i = 1;
		while(args[i] != NULL){
			rmdir(args[i]);
			printf("%s\t%s\n", "Removed file : ", args[i]);
			i++;
		}
	}
	return 1;
}

int launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
    	char* app = malloc(sizeof(char)*2);
    	app[0] = '.';
    	app[1] = '/';
    	strcat(app,args[0]);
    	args[0] = app;
    	if(execvp(args[0],args) == -1)
    		printf("%s\n", "There is no such command");
    }
    else
    	printf("%s\t%s\n", "Executing the command: ", args[0]);
    return 1;
    exit(EXIT_FAILURE);
  } 
  else if (pid < 0) {
  	printf("%s\n", "Could not create fork");
  	return 1;
  }
  else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int parsePipe(char** args){
	int i = 0;
	while(args[i] != NULL){
		if(strcmp(args[i],"|") == 0){
			printf("%s\n", "There is piping");
			return 0;
		}
		i++;
	}
	return 1;
}

int parseRed(char** args){
	int i = 0;
	while(args[i] != NULL){
		if(strcmp(args[i],">") == 0){
			printf("%s\n", "There is redirecting");
			return 0;
		}
		else if(strcmp(args[i],"<") == 0){
			printf("%s\n", "There is redirecting");
			return 1;
		}
		i++;
	}
	return 2;
}

int redOut(char** parsed, char** parsedpipe){
	pid_t pid = fork();
	if(pid == -1){
		printf("Failed to Fork \n");
		return 1;
	}
	else if(pid == 0){
		printf("Trying to shift output\n");
		int fd = open(parsedpipe[0],O_WRONLY);
		dup2(fd,1);
		close(fd);
		if (execvp(parsed[0], parsed) < 0) { 
	        char* app = malloc(sizeof(char)*2);
			app[0] = '.';
			app[1] = '/';
			strcat(app,parsed[0]);
			parsed[0] = app;
			if(execvp(parsed[0],parsed) == -1){
		       	printf("Could not execute command 1..\n");
		       	printf("%s\n", "Are you sure the files are present?");
			}
			return 1;
	    }
	    else
	    	printf("%s\n","Shifted output successfully" );
	    return 1;
		exit(0);
	}
	else{
		wait(NULL);
		return 1;
	}
	return 1;
}

int redIn(char** parsed, char** parsedpipe){
	pid_t pid = fork();
	int fd;
	if(pid == -1){
		printf("Failed to Fork \n");
		return 1;
	}
	else if(pid == 0){
		printf("Trying to get input\n");
		fd = open(parsedpipe[0],O_RDONLY);
		dup2(fd,0);
		close(fd);
		if (execvp(parsed[0], parsed) < 0) { 
	        char* app = malloc(sizeof(char)*2);
			app[0] = '.';
			app[1] = '/';
			strcat(app,parsed[0]);
			parsed[0] = app;
			if(execvp(parsed[0],parsed) == -1){
		       	printf("Could not execute command 1..\n");
		       	printf("%s\n", "Are you sure the files are present?");
		    }
	    }
		exit(0);
	}
	else{
		wait(NULL);
		return 1;
	}
	return 1;
}

int execArgsPiped(char** parsed, char** parsedpipe) 
{ 
	int pipefd[2];  
    pid_t p1, p2; 
  
    if (pipe(pipefd) < 0) { 
        printf("\nPipe could not be initialized"); 
        return 1; 
    } 
    p1 = fork(); 
    if (p1 < 0) { 
        printf("\nCould not fork"); 
        return 1; 
    } 
  
    if (p1 == 0) {
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
  
  		if (execvp(parsed[0], parsed) < 0) { 
	        char* app = malloc(sizeof(char)*2);
			app[0] = '.';
			app[1] = '/';
			strcat(app,parsed[0]);
			parsed[0] = app;
			if(execvp(parsed[0],parsed) == -1){
		       	printf("Could not execute command 1..\n");
		        printf("%s\n", "Are you sure the file exits??");
		        exit(0);
			}
	    }
	    return 1;
    } else { 
        p2 = fork(); 
  
        if (p2 < 0) { 
            printf("\nCould not fork"); 
            return 1; 
        } 
        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 
            if (execvp(parsedpipe[0], parsedpipe) < 0) { 
                char* app = malloc(sizeof(char)*2);
				app[0] = '.';
				app[1] = '/';
				strcat(app,parsedpipe[0]);
			    parsedpipe[0] = app;
			    if(execvp(parsedpipe[0],parsedpipe) == -1){
	               printf("Could not execute command 2..\n"); 
	               printf("%s\n", "Are you sure the file exits??");
	               exit(0);
	           }
	    return 1;
        } 
        } else { 
            wait(NULL); 
            wait(NULL); 
            return 1;
        } 
    } 
} 

void parse(char** args, char** parsed, char** parsedpipe){
	int i = 0;
	int abhi = 0;
	int j = 0;
	while(args[i] != NULL){
		if(strcmp(args[i],"|") == 0){
			abhi = 1;
			j = 0;
			i++;
			continue;
		}
		if(abhi == 0){
			parsed[j] = args[i];
			i++;
			j++;
		}else{
			parsedpipe[j] = args[i];
			i++;
			j++;
		}
	}
}

void parse2(char** args, char** parsed, char** parsedpipe){
	int i = 0;
	int abhi = 0;
	int j = 0;
	while(args[i] != NULL){
		if(strcmp(args[i],">") == 0 || strcmp(args[i],"<") == 0){
			abhi = 1;
			j = 0;
			i++;
			continue;
		}
		if(abhi == 0){
			parsed[j] = args[i];
			i++;
			j++;
		}else{
			parsedpipe[j] = args[i];
			i++;
			j++;
		}
	}
}

int execute(char** args){
	char **parsed,**parsedpipe;
	parsed = malloc(sizeof(char*) * 1000);
	parsedpipe = malloc(sizeof(char*) * 1000);
	if(args[0] == NULL)
		return 1;
	else if(strcmp(args[0],"mkdir") == 0)
		return shell_mkdir(args);
	else if(strcmp(args[0],"rmdir") == 0)
		return shell_rmdir(args);
	else if(strcmp(args[0],"cd") == 0)
		return cd(args);
	else if(strcmp(args[0],"pwd") == 0)
		return pwd(args);
	else if(strcmp(args[0],"exit") == 0)
		return 0;
	else{
		int exec = parsePipe(args);
		if(exec == 0){
			parse(args,parsed,parsedpipe);
			return execArgsPiped(parsed,parsedpipe);
		}
		else{
			int l =  parseRed(args);
			if(l == 0 || l== 1)
				parse2(args,parsed,parsedpipe);
			if(l == 0)
				return redOut(parsed,parsedpipe);
			else if (l == 1)
				return redIn(parsed,parsedpipe);
			else
				return launch(args);
		}
	}
	return 1;
}