/*
    dmx-interpreter.c is the file where all the magic happens. This is where your .dmx file gets converted into a .midi file
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "dmx-interpreter.h"

bool check_dmx_file(DMX_File *dmx)
{
    char dmx_check[12];
    char controller[12] = ".controller";
    fgets(dmx_check, 12, dmx->handler);
    if(strncmp(dmx_check, controller, 12) != 0)
    {
        return false;
    }
    return true;
};

bool parse_dmx_file(DMX_File *dmx)
{
    char dmx_code[256];
    char controller[12] = ".controller ";
    char bpm[5] = ".bpm ";
    char channel[9] = ".channel ";
    char commands[10] = ".commands\n";

    fseek(dmx->handler, 0, SEEK_SET);
    while (fgets(dmx_code, sizeof(dmx_code), dmx->handler)) {
        // printf("%s", dmx_code);
        if(strncmp(dmx_code,controller, 12) == 0){
            // TODO: Make logic to extract controller alias
            printf("Found controller: obey70\n");
            //strncpy(dmx->controller, "obey70", 7);
        }
        if(strncmp(dmx_code,bpm, 5) == 0){
            // TODO: Make logic to extract bpm
            printf("Found BPM: 100\n");
            dmx->bpm = 100;
        }
        if(strncmp(dmx_code,channel,9) == 0){
            // TODO: Make logic to extract channel
            printf("Found Channel: 4\n");
            dmx->channel = 4;
        }
        if(strncmp(dmx_code, commands, 10) == 0){
            // TODO: Make logic parse commands from extracted alias
            printf("Found commands section\n");
        }
    }
    return true;
}