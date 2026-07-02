#include <stdlib.h>
#include <stdio.h>

#include <midi.h>
#include "dmx-interpreter.h"
#include "midi-converter.h"
#include "../version.h"

int main(int argc, char **argv)
{
    DMX_File dmx;
    if(argc <= 2){
        printf("Usage: dmxc [Input DMX File] [Output Midi File]\n");
        return 0;
    }
    printf("DMX Lang v%i.%i.%i-%s\n", DMXLANG_VERSION_MAJOR, DMXLANG_VERSION_MINOR, DMXLANG_VERSION_PATCH, DMX_LANG_COMPILED_OS);
    dmx.handler = fopen(argv[1], "r");
    if(!dmx.handler)
    {
        printf("Failed to open DMX file\n");
        return -1;
    }
    printf("Validating DMX FIle\n");
    if(!check_dmx_file(&dmx))
    {
        printf("Input file is not a valid DMX File\n");
        return -1;
    }
    printf("Valid DMX File\n");

    if(!parse_dmx_file(&dmx))
    {
        printf("Failed to parse DMX File\n");
        return -1;
    }
    printf("Parsed DMX File\n");
    printf("Writing Midi File\n");
    if(!write_midi_file(argv[2])){
        printf("Failed to Write Midi File\n");
    }
    printf("Wrote Midi File\n");
    fclose(dmx.handler);
}