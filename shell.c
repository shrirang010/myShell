#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#define ANSI_BACKGROUND_BLACK "\x1b[40m"

int checkforCD_command(char *user_Arguement)
{
    char *cmd = "cd";
    if (!strcmp(cmd, user_Arguement))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
char *changeCWD(char *user_arguement2)
{
    char path[100];

    if (!chdir(user_arguement2))
    {
        return getcwd(path, 100);
    }
    else
    {
        return NULL;
    }
}
char *getpath(char *command, char *path)
{
    char *newstr = (char *)malloc(1024 * sizeof(char));
    strcpy(newstr, path);
    strcat(newstr, "/");
    strcat(newstr, command);
    return newstr;
}
char *find_valid_path(char *command, char *PATH)
{
    char *validpath = (char *)malloc(1024 * sizeof(char));
    char *localpath = (char *)malloc(1024 * sizeof(char));

    strcpy(localpath, PATH);
    char *ptoken = strtok(localpath, ":");

    while (ptoken != NULL)
    {
        validpath = getpath(command, ptoken);
        if (!access(validpath, F_OK))
        {
            return validpath;
        }
        ptoken = strtok(NULL, ":");
    }
    return NULL;
}
int checkforPATH_command(char *user_arguement)
{
    int i = 0;
    char *str = "PATH=";
    if (!strncmp(str, user_arguement, 5))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
char *changePATH(char *PATH, char *userinput)
{
    char *temp = (char *)malloc(1024 * sizeof(char));
    strcpy(temp, userinput);
    PATH = temp + 5;
    return PATH;
}

int checkforPS1_command(char *user_arguement1)
{
    char *temp = "PS1=\"";
    char *lastchar = "\"";
    int len, i;
    if (!strncmp(temp, user_arguement1, 5)) // checking for PS1="
    {
        len = (int)strlen(user_arguement1);
        if (!strcmp(lastchar, user_arguement1 + len - 1) && len > 6) // checking for last " from PS1="abcd" and  total length is more than 6 so that there is data in " "
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}
char *set_PROMPT(char *user_arguement1, char *prompt, int *promptset)
{
    int i = 5; // iterator from where real prompt name starts in PS1 command
    int j = 0;
    char *arg1 = (char *)malloc(100 * sizeof(char));

    char temp[strlen(prompt)];
    for (int k = 0; k < strlen(prompt); k++)
    {
        temp[k] = '\0';
    }
    strncpy(prompt, temp, (int)strlen(prompt));
    strcpy(arg1, user_arguement1);
    for (i; i < strlen(arg1) - 1; i++)
    {
        prompt[j] = arg1[i];
        j++;
    }
    *promptset = 1;
    if (!strcmp("\\w$", prompt))
    {
        prompt = getcwd(prompt, 100);
        *promptset = 0;
    }
    return prompt;
}
int checkforOutput_Redirection(char *user_command[])
{
    char *ptr = ">";
    int index; // the index where > is present
    int i = 0;
    while (user_command[i] != NULL)
    {
        if (!strcmp(ptr, user_command[i]) && i > 0)
        {
            index = i;
            return index;
        }
        i++;
    }
    return INT_MIN;
}
void Output_Redirection(char *arguements[], int index)
{
    int fd = dup(1); // duplicating file descriptor 1 ,fd will be 3 ,which will also point to the screen as of 1
    close(1);
    int fp = open(arguements[index + 1], O_WRONLY); // fp will be 1 and point to screen
    if (fp == -1)
    {
        int new_fd = dup(fd); // as fd[1] points to NULL ,new_fd will occupy fd[1] and point to where fd is pointing i.e on the screen
        close(fd);
        printf("unable to open file:%s\n", arguements[index + 1]);
        exit(0);
    }
    arguements[index] = NULL;
    arguements[index + 1] = NULL;
    return;
}
int checkforInput_Redirection(char *user_command[])
{
    char *ptr = "<";
    int i = 0;
    int index;
    while (user_command[i] != NULL)
    {
        if (!strcmp(ptr, user_command[i]) && i > 0) // i > 0 condition required so that < is not first arguement
        {
            index = i;
            return index;
        }
        i++;
    }
    return INT_MAX;
}
void InputRedirection(char *user_command[], int index) // index is the the index of user_arguement array where < was found
{
    close(0);
    int fd = open(user_command[index + 1], O_RDONLY);
    if (fd == -1)
    {
        printf("unable to open file%s\n", user_command[index + 1]);
        exit(0);
    }

    return;
}
int checkforHistory_command(char*argument[])
{
    if(!strcmp(argument[0],"History") && argument[1]== NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void printHistory()
{
    char line[1000];
    FILE*fp=fopen("History.txt", "r");
    while(fgets(line,strlen(line),fp)!=NULL)
    {
        printf("%s\n",line);
    }
    return ;
}
void Addto_History(char*arguments[],int argcount)
{
    FILE* fp;
    int j=0;
    fp=fopen("History.txt","a");
    while(j!=argcount)
    {   
        fprintf(fp,arguments[j],strlen(arguments[j]));
        j++;
    }
    fprintf(fp, "\n");
    fclose(fp);
}
int main()
{   
    char *PATH = getenv("PATH");
    strcat(PATH,":");
    strcat(PATH,".");
    char *path;
    char address[PATH_MAX];
    char commandinput[200];
    int argcount = 0;
    int i;
    int ret;
    char *arguements[100];
    char *token;
    char *cwd;
    int status;
    char *prompt = (char *)malloc(1024 * sizeof(char));
    int promptSet = 0;        // flag to check if Prompt has been changed or is default (promptSet = 1 means  prompt changed , 0 means prompt has not been changed )
    int fd;
                            printf("\033[1;35m");
                            printf("\n\t\t\t\t\t\t\tWelcome to my Shell\n");
    while (1)

    {

        if (!promptSet)
        {
            getcwd(address, sizeof(address));
            printf("\033[1;36m");
            printf("%s $", address);
        }
        else
        {
            printf("\033[1;36m");
            printf("%s $", prompt);
        }

        printf("\033[0;37m");
        scanf("%[^\n]%*c", commandinput);
        i = 0;
        token = NULL;
        token = strtok(commandinput, " ");

        if (!strcmp(token, "exit"))
        {   
            printf("\033[1;34m");            
            printf("\t\t\t\t\t\t\tShell Exited Successfully !\n");
            return 0;
        }

        while (token != NULL)
        {
            arguements[i] = token;
            i++;
            token = strtok(NULL, " ");
        }
        argcount = i;
        arguements[i] = NULL;

        Addto_History(arguements,argcount);     //Add command to History 
        
        if(checkforHistory_command(arguements)) //Check for History command
        {
            printHistory();
            argcount,i=0;
            token,path=NULL;
            continue;
        }

        if (checkforCD_command(arguements[0]))
        {
            cwd = changeCWD(arguements[1]);
            if (cwd == NULL)
            {
                printf("%s: No such file or Directory\n", arguements[1]);
            }
            argcount, i = 0;
            token, path = NULL;
            continue;
        }

        if (checkforPS1_command(arguements[0]))
        {
            prompt = set_PROMPT(arguements[0], prompt, &promptSet);
            i, argcount = 0;
            token, path = NULL;
            continue;
        }

        if (checkforPATH_command(arguements[0]))
        {
            PATH = changePATH(PATH, arguements[0]);
            token, path = NULL;
            argcount, i = 0;
            continue;
        }
        if(!strncmp("$PATH",arguements[0],5))  // PATH$ command prints current PATH environment
        {
            printf("%s\n",PATH);
            token,path = NULL;
            argcount, i = 0;
            continue;
        }

        path = find_valid_path(arguements[0], PATH);
        if (path == NULL)
        {
            printf("%s:command not found in path\n",arguements[0]);
            token,path = NULL;
            argcount, i = 0;
            continue;
        }

        int p = fork();
        if (p == 0)
        {
            int val1 = checkforOutput_Redirection(arguements); // function returns index of > if present else returns INT_MIN
            if (val1 != INT_MIN)
            {
                Output_Redirection(arguements, val1);  // val1 is index in the arguement array where > was found 
            }

            int val2 = checkforInput_Redirection(arguements); // function returns index of  if present else returns INT_MAX
            if (val2 != INT_MAX)
            {
                InputRedirection(arguements, val2);   // val2 is index in the arguement array where < was found 
            }

            ret = execv(path, arguements);

            if (ret == -1)
            {
                printf("%s:command not found\n", arguements[0]);
                exit(0);
            }
        }
        else
        {
            wait(0);
            argcount, i = 0;
            token, path = NULL;
        }
    }
    return 0;
}
