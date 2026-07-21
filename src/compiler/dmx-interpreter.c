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

void strip_comment(char *s)
{
    char *semi = strchr(s, ';');
    if(semi)
        *semi = 0;
}

bool check_dmx_file(DMX_File *dmx)
{
    char dmx_check[12];
    char controller[12] = ".controller";
    if(!fgets(dmx_check, 12, dmx->handler))
        return false;
    if(strncmp(dmx_check, controller, 12) != 0)
    {
        return false;
    }
    return true;
};

bool load_dmxd_file(DMXD_File *dmxd)
{
    char dmxd_line[256];
    int cap = 0;
    while(fgets(dmxd_line, sizeof(dmxd_line), dmxd->handler))
    {
        if(sscanf(dmxd_line, ".alias%63s", dmxd->alias) == 1)
        {
            printf("Found Controller alias %s\n", dmxd->alias);
            continue;
        }

        char *eq = strchr(dmxd_line, '=');
        if(!eq)
            continue;

        char keypart[200];
        size_t klen = (size_t)(eq-dmxd_line);
        if(klen >= sizeof(keypart))
            klen = sizeof(keypart) - 1;

        strncpy(keypart, dmxd_line, klen);
        keypart[klen]= 0;

        char *colon = strchr(keypart, ':');
        if(colon)
            *colon = 0;

        trim(keypart);
        if(keypart[0] == 0)
            continue;

        char name[200] = {0}, rest[64] = {0};
        int n = sscanf(keypart, "%32s %63s", name, rest);
        if(name[0] == 0)
            continue;

        int base_value = 0;
        sscanf(eq+1, "%d", &base_value);

        DMXD_Command command;
        memset(&command, 0, sizeof(DMXD_Command));
        snprintf(command.name, sizeof(command.name), "%s", name);
        command.base = base_value;

        int bank = 0, div = 0;
        if(n == 2 && sscanf(rest, "%d-x/%d", &bank, &div) == 2)
        {
            command.mode = COMMAND_RANGE;
            command.bank = bank;
            command.div = div;
        } else {
            command.mode = COMMAND_EXACT;
            command.num = (n == 2) ? atoi(rest) : -1;
        }

        if(dmxd->command_count >= cap)
        {
            int new_cap = cap ? cap * 2 : 16;
            DMXD_Command *grown = realloc(dmxd->commands, sizeof(DMXD_Command) * new_cap);
            if(!grown)
            {
                printf("Failed to allocate memory for command: %s\n", name);
                return false;
            }
            dmxd->commands = grown;
            cap = new_cap;
        }

        dmxd->commands[dmxd->command_count++] = command;
        printf("Found Command: %s\n", name);
    }
    return true;
};

typedef struct InstrList
{
    DMX_Instruction *items;
    int count;
    int cap;
} InstrList;

bool list_push(InstrList *list, DMX_Instruction instr)
{
    if(list->count >= list->cap)
    {
        int new_cap = list->cap ? list->cap * 2 : 16;
        DMX_Instruction *grown = realloc(list->items, sizeof(DMX_Instruction) * new_cap);
        if(!grown)
            return false;
        list->items = grown;
        list->cap = new_cap;
    }
    list->items[list->count++] = instr;
    return true;
}

bool list_append(InstrList *dst, const InstrList *src)
{
    for(int i = 0; i < src->count; i++)
    {
        if(!list_push(dst, src->items[i]))
            return false;
    }
    return true;
}

bool parse_block(char (*lines)[256], int nlines, int *idx, InstrList *out, bool require_end)
{
    while(*idx < nlines)
    {
        char *line = lines[*idx];

        char verb[32] = {0};
        char operand[64] = {0};
        int parts = sscanf(line, "%31s %63s", verb, operand);
        if(parts < 1)
        {
            (*idx)++;
            continue;
        }

        if(strcmp(verb, "end") == 0 && require_end)
        {
            (*idx)++;
            return true;
        }

        if(strcmp(verb, "loop") == 0)
        {
            int count = 0;
            sscanf(operand, "%d", &count);
            (*idx)++;

            InstrList body = {0};
            if(!parse_block(lines, nlines, idx, &body, true))
            {
                free(body.items);
                return false;
            }
            for(int r = 0; r < count; r++)
            {
                if(!list_append(out, &body))
                {
                    free(body.items);
                    return false;
                }
            }
            free(body.items);
            continue;
        }

        if(strcmp(verb, "delta") == 0)
        {
            DMX_Instruction instr = { .kind = INSTR_DELAY, .arg1 = -1, .arg2 = -1, .seconds = 0 };
            sscanf(operand, "%d", &instr.seconds);
            if(!list_push(out, instr))
                return false;
            (*idx)++;
            continue;
        }

        DMX_Instruction instr = { .kind = INSTR_LIGHT, .arg1 = -1, .arg2 = -1, .seconds = 0 };
        snprintf(instr.command, sizeof(instr.command), "%s", verb);
        int a = 0, b = 0;
        if(sscanf(operand, "%d-%d", &a, &b) == 2)
        {
            instr.arg1 = a;
            instr.arg2 = b;
        }
        else if(sscanf(operand, "%d", &a) == 1)
        {
            instr.arg1 = a;
        }
        if(!list_push(out, instr))
            return false;
        (*idx)++;
    }

    if(require_end)
    {
        printf("Error: Missing 'end' for 'loop' block\n");
        return false;
    }
    return true;
}

bool parse_dmx_file(DMX_File *dmx)
{
    fseek(dmx->handler, 0, SEEK_SET);

    char raw[256];
    bool in_commands = false;
    char (*lines)[256] = NULL;
    int nlines= 0, cap = 0;

    while(fgets(raw, sizeof(raw), dmx->handler))
    {
        strip_comment(raw);
        trim(raw);
        if(raw[0] == 0)
            continue;

        if(!in_commands){
            if(sscanf(raw, ".controller %63s", dmx->controller) == 1)
            {
                printf("Found controller: %s\n", dmx->controller);
            }
            else if(sscanf(raw, ".bpm %d", &dmx->bpm) == 1)
            {
                printf("Found BPM: %d\n", dmx->bpm);
            }
            else if(sscanf(raw, ".channel %d", &dmx->channel) == 1)
            {
                printf("Found Channel: %d\n", dmx->channel);
            }
            else if(strcmp(raw, ".commands") == 0)
            {
                in_commands = true;
                printf("Found commands section\n");
            }
            continue;
        }

        if(nlines >= cap)
        {
            int new_cap = cap ? cap * 2: 16;
            char(*grown)[256] = realloc(lines, sizeof(*lines) * new_cap);
            if(!grown)
            {
                free(lines);
                printf("Failed to allocate memory for dmx command\n");
                return false;
            }
            lines = grown;
            cap = new_cap;
        }
        snprintf(lines[nlines++], 256, "%s", raw);
    }

    if(!in_commands)
    {
        free(lines);
        printf("Error: Missing '.commands' section\n");
        return false;
    }

    InstrList program = {0};
    int idx = 0;
    bool ok = parse_block(lines, nlines, &idx, &program, false);
    free(lines);

    if(!ok){
        free(program.items);
        return false;
    }
    dmx->instructions = program.items;
    dmx->instruction_count = program.count;
    printf("Compiled %d instruction(s}\n", program.count);
    return true;
}

bool resolve_note(DMXD_File *dmxd, DMX_Instruction *instr, int *note_out)
{

    for(int i = 0; i < dmxd->command_count; i++)
    {
        const DMXD_Command *c = &dmxd->commands[i];
        if(strcmp(c->name, instr->command)!=0)
            continue;

        if(c->mode == COMMAND_RANGE)
        {
            if(instr->arg1 != c->bank)
                continue;

            int slot = instr->arg2;
            if(slot < 1 || (c->div > 0 && slot > c->div))
                continue;

            *note_out = c->base + (slot - 1);
            printf("Resolved note: %d\n", *note_out);
            return true;
        }
        else
        {
            if(c->num != instr->arg1)
                continue;
            *note_out = c->base;
            printf("Resolved note: %d\n", *note_out);
            return true;
        }
    }
    return false;
}

// Just a simple wrapper for the main loop
bool visualizer_load_sequence(DMX_File *dmx, DMXD_File *dmxd)
{
    if(!check_dmx_file(dmx))
    {
        printf("Input file is not a valid DMX File\n");
        return false;
    }
    printf("Valid DMX File\n");
    printf("Loading DMXD Definitions\n");
    if(!load_dmxd_file(dmxd))
    {
        printf("Failed to Load DMX Definitions\n");
        free(dmxd->commands);
        return false;
    }
    printf("Parsing DMX File\n");
    if(!parse_dmx_file(dmx))
    {
        printf("Failed to parse DMX File\n");
        free(dmxd->commands);
        return false;
    }
    printf("Parsed DMX File\n");
    if(dmxd->alias[0] && dmx->controller[0] && strcmp(dmx->controller, dmxd->alias) != 0)
    {
        printf("Warning: DMX controller '%s' does not match DMXD alias '%s'\n",
               dmx->controller, dmxd->alias);
        free(dmx->instructions);
        free(dmxd->commands);
        return false;
    }
    return true;
}