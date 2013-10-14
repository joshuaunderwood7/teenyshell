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
    int createPipe(const int argc1, const char* path1, char* argv1[], const int argc2, const char* path2, char* argv2[]);
    int createProcess(const int argc, const char* path, char* argv[]);
    int createFile(const int argc1, const char* path1, char* argv1[], char* filename);

    bool changeDir(const char* newDir);


}
#endif
