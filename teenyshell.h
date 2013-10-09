#ifndef TEENYSHELL_H
#define TEENYSHELL_H

#include <string>
#include <vector>

namespace teenyshell
{
    char* getCommandc(void);
    char* doesProgramExistc(const char* progName);

    char** parsePathc(char ** paths);
    char** parseCommandc(char* command, int &argc, char* argv[]);

    int makeSystemCall(const int argc, char* argv[]);
    int createProcess(const int argc, const char* path, char* argv[]);

    bool changeDir(const char* newDir);


}
#endif
