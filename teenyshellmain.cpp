#include <iostream>
#include "teenyshell.h"
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

using namespace teenyshell;
 
int main(int argc, const char *argv[])
{
    
    std::cout << "Welcome to teenyshell\n"
              << "Written by Joshua Underwood\n"
              << "\n"
              << "******************************\n"
              << "*Warning, the left carrot \"<\"*\n"
              << "*will cause a seg fault      *\n"
              << "*and the pipe \"|\" does not   *\n"
              << "*work, should not crash.     *\n"
              << "*wc and grep also do not work*\n"
              << "*and will crash shell.       *\n"
              << "******************************\n"
              << "\n";

    char command[1024];
    int cargc;
    char** cargv;
    command[0] = 0;

    
    while((strcmp(command, "exit")!=0))
    {
        char* temp = getCommandc();
        strcpy(command, temp);
        delete temp;

        if((strcmp(command, "exit")!=0))
        {
            cargv = parseCommandc(command, cargc, cargv);
            makeSystemCall(cargc, cargv);

            //clean up memory
            int i;
            for (i = 0; i < cargc; i++) {
                delete cargv[i];
            }
            delete cargv;
        }
    }

    return EXIT_SUCCESS;
}   

