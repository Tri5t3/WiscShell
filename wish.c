#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#define DELIM " \n"

// Global variables
int strSize = 64;
int argBuffer = 99;
int pathBuffer = 64;

char **lineHandler(char *line, char *ret[])
{
    char input[argBuffer][strSize];
    char *container;
    int index = 0;
    for (int i = 0; i < argBuffer; i++)
    {
        strcpy(input[i], "");
    }
    while ((container = strsep(&line, DELIM)) != NULL)
    {
        if (*container == '\0')
        {
            continue;
        }
        strcpy(input[index], container);
        index++;
    }

    for (int i = 0; i < index; i++)
    {
        if (strlen(input[i]) > 1)
        {
            if (strstr(input[i], ">") != NULL)
            {
                for (int j = 1; j < index - i; j++)
                {
                    strcpy(input[i + j + 2], input[i + j]);
                }
                strcpy(input[i + 1], ">");
                
                char badInput[strSize];
                char *str;
                strcpy(badInput, input[i]);
                strcpy(input[i], strtok(badInput, ">"));
                str = strtok(NULL, ">");
                strcpy(input[i + 2], str);
            }
        }
    }

    int strLen = 0;
    for (int i = 0; strcmp(input[i], "") != 0; i++)
    {
        ret[i] = input[i];
        strLen++;
    }
    ret[strLen] = NULL;

    return ret;
}

int getLen(char *ret[])
{
    int retV = 0;
    for (int i = 0; ret[i] != NULL; i++)
    {
        retV++;
    }
    return retV;
}

int findRedir(char *input[])
{
    int len = getLen(input);
    int ret = -1;
    for (int i = 0; i < len; i++)
    {
        char *cmp;
        cmp = strdup(input[i]);
        if (ret == -1)
        {
            if (strcmp(cmp, ">") == 0)
            {
                ret = i;
            }
        }
        else
        {
            if (strcmp(cmp, ">") == 0)
            {
                ret = -2;
            }
        }
    }
    return ret;
}

void do_Fork(char *input[], int pathCount, char *_1_d_path[])
{
    int len = getLen(input);
    char apdPath[pathCount][strSize];
    for (int i = 0; i < pathCount; i++)
    {
        strcpy(apdPath[i], "");
    }
    for (int i = 0; i < pathCount; i++)
    {
        strcat(apdPath[i], _1_d_path[i]);
        strcat(apdPath[i], "/");
        strcat(apdPath[i], input[0]);
    }

    int rc = fork();
    if (rc < 0)
    {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
    else if (rc == 0) // child
    {
        // search executable
        int exeIndex = -1;
        for (int i = 0; i < pathCount; i++)
        {
            if (access(apdPath[i], X_OK) == 0)
            {
                exeIndex = i;
                break;
            }
        }
        if (exeIndex == -1) // not found
        {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        else
        {
            int redir = findRedir(input);
            if (redir == -2)
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            if (redir > 0 && input[redir + 1] == NULL)
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            if (redir > 0 && redir != len - 2)
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }

            if (redir > 0 && input[redir + 1] != NULL)
            {
                close(STDOUT_FILENO);
                open(input[redir + 1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                input[redir] = NULL;
            }

            execv(apdPath[exeIndex], input);
        }
    }
    else // parent
    {
        wait(NULL);
    }
}

int main(int argc, char *argv[])
{
    char path[pathBuffer][strSize];
    for (int i = 0; i < pathBuffer; i++)
    {
        strcpy(path[i], "");
    }
    strcpy(path[0], "/bin");
    int pathCount = 1;
    char *_1_D_path[pathBuffer];
    for (int i = 0; strcmp(path[i], "") != 0; i++)
    {
        _1_D_path[i] = path[i];
    }

    if (argc == 1) // basic mode
    {
        while (1)
        {
            printf("wish> ");
            char *input[argBuffer];

            char *line = NULL;
            size_t len = 0;
            ssize_t nread;
            if ((nread = getline(&line, &len, stdin)) != -1)
            {
                lineHandler(line, input);
            }
            int argCount = getLen(input);

            if (argCount == 0)
                continue;
            if (strcmp(input[0], "exit") == 0)
            {
                if (argCount != 1)
                {
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    continue;
                }
                exit(0);
            }
            if (strcmp(input[0], "cd") == 0)
            {
                if (argCount != 2)
                {
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    continue;
                }
                if (chdir(input[1]) != 0)
                {
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    continue;
                }
                continue;
            }
            // path
            if (strcmp(input[0], "path") == 0)
            {
                if (argCount == 1)
                {
                    for (int i = 0; i < pathCount; i++)
                    {
                        strcpy(path[i], "");
                    }
                    pathCount = 0;
                }
                else
                {
                    for (int i = 1; i < argCount; i++)
                    {
                        strcpy(path[i - 1], input[i]);
                    }
                    pathCount = argCount - 1;
                }
                for (int i = 0; strcmp(path[i], "") != 0; i++)
                {
                    _1_D_path[i] = path[i];
                }
                continue;
            }
            if (strcmp(input[0], "loop") == 0)
            {
                if (argCount < 3)
                {
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    continue;
                }
                int loopCount = atoi(input[1]);
                if (loopCount < 1)
                {
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    continue;
                }
                int loopIndicator = -1;
                for (int i = 0; i < argCount; i++)
                {
                    char *cmp;
                    cmp = strdup(input[i]);
                    if (strcmp(cmp, "$loop") == 0)
                    {
                        loopIndicator = i;
                    }
                }

                // looping
                for (int i = 0; i < loopCount; i++)
                {

                    char *loopInput[argBuffer];
                    // nullify
                    for (int k = 0; k < argBuffer; k++)
                    {
                        loopInput[k] = NULL;
                    }

                    for (int loop = 2; loop < argCount; loop++)
                    {
                        loopInput[loop - 2] = input[loop];
                    }

                    if (loopIndicator != -1)
                    {
                        char strRepofInt[12];
                        sprintf(strRepofInt, "%d", i + 1);
                        loopInput[loopIndicator - 2] = strRepofInt;
                    }
                    do_Fork(loopInput, pathCount, _1_D_path);
                }
                continue;
            }
            // fork
            do_Fork(input, pathCount, _1_D_path);
            continue;
        }
    }
    if (argc > 1) // batch mode
    {

        for (int i = 1; i < argc; i++) // each file
        {
            FILE *file;
            file = fopen(argv[i], "r+");
            if (file == NULL)
            {
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            char *line = NULL;
            size_t len = 0;
            ssize_t read;

            // each line
            while ((read = getline(&line, &len, file)) != -1)
            {
                char *input[argBuffer];
                lineHandler(line, input);
                int argCount = getLen(input);

                // simple three
                if (argCount == 0)
                    continue;
                if (strcmp(input[0], "exit") == 0)
                {
                    if (argCount != 1)
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        if (argc > 2)
                        {
                            exit(1);
                        }
                        continue;
                    }
                    exit(0);
                }
                if (strcmp(input[0], "cd") == 0)
                {
                    if (argCount != 2)
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        if (argc > 2)
                        {
                            exit(1);
                        }
                        continue;
                    }
                    if (chdir(input[1]) != 0)
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        if (argc > 2)
                        {
                            exit(1);
                        }
                        continue;
                    }
                    continue;
                }
                // path
                if (strcmp(input[0], "path") == 0)
                {
                    if (argCount == 1)
                    {
                        for (int i = 0; i < pathCount; i++)
                        {
                            strcpy(path[i], "");
                        }
                        pathCount = 0;
                    }
                    else
                    {
                        for (int i = 1; i < argCount; i++)
                        {
                            strcpy(path[i - 1], input[i]);
                        }
                        pathCount = argCount - 1;
                    }
                    for (int i = 0; strcmp(path[i], "") != 0; i++)
                    {
                        _1_D_path[i] = path[i];
                    }
                    continue;
                }
                if (strcmp(input[0], "loop") == 0)
                {
                    if (argCount < 3)
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        if (argc > 2)
                        {
                            exit(1);
                        }
                        continue;
                    }
                    int loopCount = atoi(input[1]);
                    if (loopCount < 1)
                    {
                        char error_message[30] = "An error has occurred\n";
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        if (argc > 2)
                        {
                            exit(1);
                        }
                        continue;
                    }
                    int loopIndicator = -1;
                    for (int i = 0; i < argCount; i++)
                    {
                        char *cmp;
                        cmp = strdup(input[i]);
                        if (strcmp(cmp, "$loop") == 0)
                        {
                            loopIndicator = i;
                        }
                    }

                    // looping
                    for (int i = 0; i < loopCount; i++)
                    {
                        char *loopInput[argBuffer];
                        // nullify
                        for (int k = 0; k < argBuffer; k++)
                        {
                            loopInput[k] = NULL;
                        }

                        for (int loop = 2; loop < argCount; loop++)
                        {
                            loopInput[loop - 2] = input[loop];
                        }

                        if (loopIndicator != -1)
                        {
                            char strRepofInt[12];
                            sprintf(strRepofInt, "%d", i + 1);
                            loopInput[loopIndicator - 2] = strRepofInt;
                        }
                        do_Fork(loopInput, pathCount, _1_D_path);
                    }
                    continue;
                }
                // fork
                do_Fork(input, pathCount, _1_D_path);
                continue;
            }
        }
    }
}
