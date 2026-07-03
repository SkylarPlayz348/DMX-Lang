#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum CommandMode
{
    COMMAND_EXACT,
    COMMAND_RANGE
} CommandMode;

typedef struct DMX_File
{
    int bpm;
    int channel;
    char *controller;
    FILE *handler;
} DMX_File;

typedef struct DMXD_Command
{
    char name[32];
    CommandMode mode;
    int num;
    int bank;
    int div;
    int base;
} DMXD_Command;

typedef struct DMXD_File
{
    DMXD_Command *commands;
    int command_count;
    FILE *handler;
} DMXD_File;

bool check_dmx_file(DMX_File *dmx);
bool load_dmxd_file(DMXD_File *dmxd);
bool parse_dmx_file(DMX_File *dmx);