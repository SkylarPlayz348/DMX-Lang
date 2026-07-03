/*
    dmx-interpreter.c is the file where all the magic happens. This is where your .dmx file gets converted into a .midi file
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>

#include "dmx-interpreter.h"

static void trim(char *s)
{
    char *start = s;
    while(isspace((unsigned char)*start)) start++;
    memmove(s, start, strlen(start) +1);
    size_t len = strlen(s);
    while(len > 0 && isspace((unsigned char)s[len-1])) s[--len] = 0;
}

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

bool load_dmxd_file(DMXD_File *dmxd)
{
    char dmxd_line[256];
    while(fgets(dmxd_line, sizeof(dmxd_line), dmxd->handler))
    {
        char *eq = strchr(dmxd_line, '=');
        if(!eq)
            continue;

        char keypart[200];
        size_t klen = (size_t)(eq-dmxd_line);
        if(klen >= sizeof(keypart))
            klen = keypart -1;

        strncpy(keypart, dmxd_line, klen);
        keypart[klen] = 0;

        char *colon = strchr(keypart, ':');
        if(colon)
            *colon = 0;

        trim(keypart);
        if(keypart[0] == 0)
            continue;

        char name[200] = {0}, rest[64] = {0};
        int n = sscanf(keypart, "%31s %63s", name, rest);
        if(name[0] == 0)
            continue;

        int base_value = 0;
        sscanf(eq+1, "%d", &base_value);

        DMXD_Command command;
        memset(&command, 0, sizeof(DMXD_Command));
        snprintf(command.name, sizeof(command.name), "%s", name);
        command.base = base_value;

        int bank, div;
        if(n == 2 && sscanf(rest,"%d-x%d", &bank, &div))
        {
            command.mode = COMMAND_RANGE;
            command.bank = bank;
            command.div = div;
        } else {
            command.mode = COMMAND_EXACT;
            command.num = (n==2)?atoi(rest) :-1;
        }

        dmxd->commands = realloc(dmxd->commands, sizeof(DMXD_Command)*(dmxd->command_count +1));
        dmxd->commands[dmxd->command_count++]=command;
        printf("Found Command: %s\n", name);
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