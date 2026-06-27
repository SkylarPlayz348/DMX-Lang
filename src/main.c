#include <stdlib.h>
#include <stdio.h>

#include "version.h"

int main(int argc, char **argv)
{
    if(argc <= 2){
        printf("Usage: dmxc [Input DMX File] [Output Midi File]");
        return 0;
    }
    printf("DMX Lang v%i.%i.%i-%s\n", DMXLANG_VERSION_MAJOR, DMXLANG_VERSION_MINOR, DMXLANG_VERSION_PATCH, DMX_LANG_COMPILED_OS);

}