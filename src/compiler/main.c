#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <midi.h>
#include "dmx-interpreter.h"
#include "midi-converter.h"
#include "../version.h"

int main(int argc, char **argv)
{
    DMX_File dmx = {0};
    DMXD_File dmxd = {0};
    if(argc <= 3){
        printf("Usage: dmxc [Input DMX File] [Input DMXD File] [Output Midi File]\n");
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
        fclose(dmx.handler);
        return -1;
    }
    printf("Valid DMX File\n");
    dmxd.handler = fopen(argv[2], "r");
    if(!dmxd.handler)
    {
        printf("Failed to Open DMXD File\n");
        fclose(dmx.handler);
        return -1;
    }
    printf("Loading DMXD Definitions\n");
    if(!load_dmxd_file(&dmxd))
    {
        printf("Failed to Load DMX Definitions\n");
        fclose(dmx.handler);
        fclose(dmxd.handler);
        free(dmx.instructions);
        free(dmxd.commands);
        return -1;
    }
    printf("Parsing DMX File\n");
    if(!parse_dmx_file(&dmx))
    {
        printf("Failed to parse DMX File\n");
        fclose(dmx.handler);
        fclose(dmxd.handler);
        free(dmx.instructions);
        free(dmxd.commands);
        return -1;
    }
    printf("Parsed DMX File\n");
    if(dmxd.alias[0] && dmx.controller[0] && strcmp(dmx.controller, dmxd.alias) != 0)
    {
        printf("Warning: DMX controller '%s' does not match DMXD alias '%s'\n", dmx.controller, dmxd.alias);
        return -1;
    }
    printf("Writing Midi File\n");
    if(!write_midi_file(argv[3], &dmx, &dmxd)){
        printf("Failed to Write Midi File\n");
        fclose(dmx.handler);
        fclose(dmxd.handler);
        free(dmx.instructions);
        free(dmxd.commands);
        return -1;
    }
    printf("Wrote Midi File\n");
    fclose(dmx.handler);
    fclose(dmxd.handler);
    free(dmx.instructions);
    free(dmxd.commands);
    return 0;
}