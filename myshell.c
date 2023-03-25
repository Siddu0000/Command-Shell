#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()

char** parseInput(char* cmd_line)
{
	// This function will parse the input string into multiple commands or a single command with comuments depending on the delimiter (&&, ##, >, or spaces).
	char* str;
	char** cmd = malloc(64*sizeof(char*));
	int pos = 0;
	while((str = strsep(&cmd_line," ")) != NULL) //strsep for separating the inputs by space
	{
        if(strlen(str) == 0) // handling null case
		{
    		continue;
    	}
		cmd[pos] = str;
		pos++;
	}
	cmd[pos] = NULL;
	return cmd;
}


void executeCommand(char** cmd)
{
    // This function will fork a new process to execute a command
	if(strcmp(cmd[0],"cd") == 0)
	{
		chdir(cmd[1]); // changing directory
		return;
	}
    else if(strcmp(cmd[0],"exit") == 0)
    {
        printf("Exiting shell...\n"); // Exiting the shell
        exit(1);
    }
	else
	{
	    int pid = fork();
		if (pid < 0) // If fork fails
		{
			exit(0); 
	    }
	    else if (pid == 0)
		{
	        // Inside child process
	        if (execvp(cmd[0], cmd) < 0)  // In case of incorrect command
		    {
	            printf("Shell: Incorrect command\n");
			    exit(1);
	        }
	    }
		else
		{
	        // Parent process
	        int pid_wait = wait(NULL);
	    }
	}
}

void executeParallelCommands(char **cmd)
{
    // This function will run multiple commands in parallel
	int i = 0;
	int first = i;
	while(cmd[i] != NULL)
	{
		while(cmd[i] != NULL && strcmp(cmd[i],"&&") != 0) // Till we encounter &&
		{
			i++;
		}
		cmd[i] = NULL;
		int pid = fork();
		if(pid < 0)
		{
			exit(1);
		}
		else if(pid == 0)
		{
            // Child process
			if(execvp(cmd[first], &cmd[first]) < 0) // In case of incorrect command
			{
				printf("Shell: Incorrect cmdmand\n");
				exit(1);
			}
		}
		i++;
		first = i;
	}

	int pid_wait = wait(NULL); // Parent process

}

void executeSequentialCommands(char** cmd)
{
	// This function will run multiple commands in sequence
	int i=0;
	int first = i;
	while(cmd[i] != NULL)
	{
		while(cmd[i]!=NULL && strcmp(cmd[i],"##") != 0) // Till we encounter ##
		{
			i++;
		}
		cmd[i] = NULL;
	 	executeCommand(&cmd[first]); // Executing sequential commands
		i++;
		first = i;
	}
}

void executeCommandRedirection(char** cmd)
{
	// This function will run a single command with output redirected to an output file specificed by user
	int i = 0;
	int first = i;
	while(cmd[i] != NULL && strcmp(cmd[i],">") != 0) // Till we encounter >
	{
		i++;
	}
	cmd[i] = NULL;
	int pid = fork();
	if(pid < 0)  // If fork fails
	{
		exit(1);
	}
	else if(pid == 0)
	{
    	if(cmd[i+1] == NULL)
			return;

        // Closing the standard output and opening the requested file
		close(STDOUT_FILENO);
		open(cmd[i+1], O_CREAT | O_WRONLY | O_APPEND);

		if(execvp(cmd[0],cmd)<0) // In case of incorrect command
		{
			printf("Shell: Incorrect command");
			exit(1);
		}
	}
	else
	{
		int pid_wait = wait(NULL); // Parent process 
	}
}

int main()
{
    // Ignores Ctrl+c, Ctrl+z
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	// Initial declarations
	long unsigned int size = 25;
	char* cmd_line;
	cmd_line = (char*)malloc(size);
	char** commands;
	int flag = 0;

	while(1)	// Achieves infinite loop untill exit is used
	{
		// Print the prompt in format :- currentWorkingDirectory$
		char dir[50];
        printf("%s$", getcwd(dir,50));

		// Accepting input using getline()
		getline(&cmd_line,&size,stdin);		

        int i = 0;
		while(cmd_line[i] != '\0')
		{
			if(cmd_line[i] == EOF || cmd_line[i] == '\n') // Changing EOF and newline for our convenience of passing the command
			{
				cmd_line[i] = '\0';
			}
			i++;
		}

		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		commands = parseInput(cmd_line);

		if(strcmp(commands[0],"exit") == 0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}

		i = 0;
		while(commands[i] != NULL) // Checking among parallel, sequential and redirectional processes
		{
			if(strcmp(commands[i],"&&") == 0) // Parallel
			{
				flag = 1;
			}
			else if(strcmp(commands[i],"##") == 0) //Sequential
			{
				flag = 2;
			}
			else if(strcmp(commands[i],">") == 0) // redirectional
			{
				flag = 3;
			}
			i++;
		}

	 	if(flag == 1)
			executeParallelCommands(commands);		// Invoked when user wants to run multiple commands parallelly (commands separated by &&)
		else if(flag == 2)
			executeSequentialCommands(commands);	// Invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(flag == 3)
			executeCommandRedirection(commands);	// Invoked when user wants to redirect the output of a single command to the output file specificed by the user (by utilizing >)
		else
			executeCommand(commands);		// Invoked to run single command

	}

	return 0;
}