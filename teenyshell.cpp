//#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "teenyshell.h"

namespace teenyshell
{

    const int MAX_PATHS = 64;
    const int STANDARD_STRING_LENGTH = 1024;

    
int makeSystemCall(const int argc, char* argv[])
{
    for (int i = 0; i < argc; i++) {
        std::cout << argv[i];
    }
    std::cout  << std::endl;

    //changing directories requires special call
    if(strcmp(argv[0], "cd")==0)
    {
        if(argc < 2) { return 1; }
        changeDir(argv[1]);
        return 0; 
    }


    char* callToMake;
    int returncode = 0;


    //if the command falls through the Teenyshell builtins make system process
    //first verify that program exists
    callToMake = doesProgramExistc(argv[0]);
    if(strcmp(callToMake, "")==0)
    { std::cerr << "Teenyshell: Command does not exist" << std::endl; return 1; }

    int fds[2];
    bool piped = false;
    int oldin, oldout;
    for (int i = 0; i < argc; i++) 
    {
        if(strcmp("|", argv[i])==0)
        {
            piped = true;
            int prev = 0;

            char* callToMakeToo;
            char** argv1;
            char** argv2;
            int argc1, argc2;

            argv1 = argv;
            argc1 = (i);
            argv[i] = NULL; //Turn pipe into null to terminate argv1

            prev = i + 1;
            argv2 = (argv + prev); 
            callToMakeToo = doesProgramExistc(*(argv + prev));
            argc2 = (argc - prev);

            returncode = createPipe(argc1, callToMake, argv1, argc2, callToMakeToo, argv2);
            delete callToMakeToo;
            i = argc;
        }
        else if(strcmp("<", argv[i])==0)
        {
            piped = true;
            int prev = 0;

            char** argv1;
            char* argv2[2];
            int argc1, argc2;

            argv1 = argv;
            argc1 = (i);
            argv[i] = NULL; //Turn carrot into null to terminate argv1

            char* callToMakeToo;
            strcpy(callToMakeToo, "/bin/cat");
            argv2[0] = callToMakeToo;
            argv2[1] = argv[i+1]; 
            argv2[2] = NULL; 
            argc2 = (3);

            returncode = createPipe(argc1, callToMake, argv1, argc2, callToMakeToo, argv2);
            i = argc;
            delete argv2[0];
        }
        else if(strcmp(">", argv[i])==0)
        {
            piped = true;
            int prev = 0;

            char** argv1;
            int argc1;

            argv1 = argv;
            argc1 = (i);
            argv[i] = NULL; //Turn carrot into null to terminate argv1

            char* filename = argv[i+1];

            returncode = createFile(argc1, callToMake, argv1, filename);
            i = argc;
        }
    }
    if(!piped) 
    { 
        //std::cout << "Inside a normal call" << std::endl;
        returncode = createProcess(argc, callToMake, argv); 
    }


    delete callToMake;
    return returncode;
}

int createPipe(const int argc1, const char* path1, char* argv1[], const int argc2, const char* path2, char* argv2[])
{
    //make the pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) 
    {
        perror("pipe");
        return 2;
    }

    //make the fork
    pid_t pid  = fork();
    if (pid == -1) // is there an error with the fork?
    {
            perror("fork"); 
            return 1;
    }


    // ---- by when you get here there will be two processes
    if (pid == 0) // child process
    {
        close(pipefd[0]);       //child close the input of the pipe
        dup2(pipefd[1], 1);     //duplicate output side of pipe to stdout
        execvp(path1, argv1);    //call the first (left of pipe)  process
        pthread_exit(0);                        //End thread when syscall is over 
    }
    else // parent process
    {
        close(pipefd[1]);       //parent close the output end of pipe
        //dup2(pipefd[0], 0);     //duplicate input side of pipe to stdin
        wait(NULL); //Wait for child process to end
        createProcess(argc2, path2, argv2);    //call the first (left of pipe) process
    }
    
    return 0;
}

int createProcess(const int argc, const char* path, char* argv[])
{
    pid_t pid  = fork();
    if (pid == -1) // is there an error with the fork?
    {
            perror("fork"); 
            return 1;
    }
    // ---- by when you get here there will be two processes
    if (pid == 0) // child process
    {
        execvp(path, argv);
        pthread_exit(0);                        //End thread when syscall is over }
    }
    else // parent process
    {
        wait(NULL); //Wait for child process to end
    }
    
    return 0;
}

int createFile(const int argc1, const char* path1, char* argv1[], char* filename)
{
    char theOutput[STANDARD_STRING_LENGTH];
    //make the pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) 
    {
        perror("pipe");
        return 2;
    }

    //make the fork
    pid_t pid  = fork();
    if (pid == -1) // is there an error with the fork?
    {
            perror("fork"); 
            return 1;
    }


    // ---- by when you get here there will be two processes
    if (pid == 0) // child process
    {
        close(pipefd[0]);       //child close the input of the pipe
        dup2(pipefd[1], 1);     //duplicate output side of pipe to stdout
        execvp(path1, argv1);    //call the first (left of pipe)  process
        pthread_exit(0);                        //End thread when syscall is over 
    }
    else // parent process
    {
        char theOutput[STANDARD_STRING_LENGTH*4];
        std::ofstream outFile;
        outFile.open(filename, std::fstream::ate);

        close(pipefd[1]);       //parent close the output end of pipe
        //dup2(0, pipefd[0]);     //duplicate input side of pipe to stdin
        wait(NULL); //Wait for child process to end
//        std::cout << "about to read from the pipe" << std::endl;
        int nbytes = read(pipefd[0], theOutput, sizeof(theOutput));
//        std::cout << theOutput  << " was read from pipe"<< std::endl;
        for (int i = 0; theOutput[i] != 0 ; i++) {
            outFile.put(theOutput[i]);
        }
    }
    
    return 0;
}

char** parsePathc(char ** paths)
{
    char pathvar[2*STANDARD_STRING_LENGTH];
    strcpy(pathvar, getenv("PATH"));
    char tempstr[STANDARD_STRING_LENGTH] = "";

    //calculates argc
    int pathc = 1;
    int pathvarLen = strlen(pathvar);

    for(int i = 0; pathvar[i] != 0; ++i)
    {
        if(pathvar[i] == ':') 
        { 
            ++pathc;
        }
    }
    //std::cout << "pathc: " << pathc << std::endl;
    
    //fills the arguments
    paths = new char*[pathc + 1];

    char token[STANDARD_STRING_LENGTH];
    char* temp;

    strcpy(token, strtok(pathvar, ":"));

    for(int i = 0; i < pathc; ++i)
    {
        paths[i] = new char[strlen(token+1)];
        std::strcpy(paths[i], token);
//        std::cout << "paths[" << i << "]: " << paths[i] << std::endl;
        temp = strtok(NULL, ":");
        if(temp != NULL) { strcpy(token, temp); }
    }
    paths[pathc] = NULL;

    return paths;
}



char* getCommandc(void)
{
    char* command = new char[STANDARD_STRING_LENGTH];
    char* PWD = getcwd(NULL, 0);

    strcpy(command, "");
    while((strcmp(command, "") == 0))
    {
        std::cout << PWD << " #!>";
        std::cin.getline(command, STANDARD_STRING_LENGTH);
        std::cin.clear();
    }
    return command;
}

char** parseCommandc(char* command, int &argc, char* argv[])
{
    //std::cin.sync();
    char tempstr[STANDARD_STRING_LENGTH] = "";

    //remove whitespace from parseCommand to keep string const
    while(strlen(command) > 0 && command[0] == ' ' || command[0] == '\t') 
    { 
//        std::cout << "Removing whitespace...\n";
        size_t n = strlen(command);         //varify that this will copy the null termination
        strncpy(tempstr, &(command[1]), n);
        strcpy(command, tempstr);
        strcpy(tempstr, "");
    } 


    //calculates argc
    argc = 1;
    int commandLength = strlen(command);

    for(int i = 0; command[i] != 0; ++i)
    {
        if(command[i] == ' ') 
        { 
            while(command[(i+1)] == ' ' || command[(i+1)] == '\t' && i != commandLength) { ++i; }
            ++argc;
        }
    }
//    std::cout << "argc: " << argc << std::endl;
    
    //fills the arguments
    argv = new char*[argc + 1];

    argv[0] = new char[STANDARD_STRING_LENGTH];
    char token[STANDARD_STRING_LENGTH];
    char* temp;

    strcpy(token, strtok(command, " \t"));

    for(int i = 0; i < argc; ++i)
    {
        argv[i] = new char[strlen(token+1)];
        std::strcpy(argv[i], token);
//        std::cout << "argv[" << i << "]: " << argv[i] << std::endl;
        temp = strtok(NULL, " \t");
        if(temp != NULL) { strcpy(token, temp); }
    }
    argv[argc] = NULL;

    return argv;
}

bool changeDir(const char* newDir)
{
    chdir(newDir);
    setenv("PWD", getcwd(NULL, 0), true);
    return true;
}


char* doesProgramExistc(const char* progName)
{
    char** paths; 
    paths = parsePathc(paths);

    struct stat st;
    char* temp = new char[STANDARD_STRING_LENGTH];
    strcpy(temp, "");

    for(int i = 0; paths[i] != NULL; ++i)
    {
        strcpy(temp, "");
        strcpy(temp, paths[i]);
        strcat(temp, "/");
        strcat(temp, progName);
//        std::cout << "The path and program are: " << temp << std::endl;
        if(stat(temp, &st) == 0)
        {
            //clean up paths.
            for (i = 0; paths[i] != NULL; i++) {
                delete paths[i];
            }
            delete paths;
            return temp;
        }
    }

    //last ditch, look for program in current folder
    strcpy(temp, "./");
    strcat(temp, progName);
    if(stat(temp, &st) == 0)
    {
        //clean up paths.
        for (int i = 0; paths[i] != NULL; i++) {
            delete paths[i];
        }
        delete paths;
        return temp;
    }

    strcpy(temp, "");
    //clean up paths.
    for (int i = 0; paths[i] != NULL; i++) {
        delete paths[i];
    }
    delete paths;
    return temp;
}

};
