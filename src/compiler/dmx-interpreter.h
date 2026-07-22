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

typedef enum DMX_InstrKind
{
    INSTR_LIGHT,
    INSTR_DELAY
} DMX_InstrKind;

typedef struct DMX_Instruction
{
    DMX_InstrKind kind;
    char command[32];
    int arg1;
    int arg2;
    int seconds;
} DMX_Instruction;

typedef struct DMX_File
{
    int bpm;
    int channel;
    char controller[64];
    DMX_Instruction *instructions;
    int instruction_count;
    FILE *handler;
} DMX_File;

typedef struct DMXD_Command
{
    char name[32];
    CommandMode mode;
    int num;
    int div;
    int base;
} DMXD_Command;

typedef struct DMXD_File
{
    char alias[64];
    DMXD_Command *commands;
    int command_count;
    FILE *handler;
} DMXD_File;

bool check_dmx_file(DMX_File *dmx);
bool load_dmxd_file(DMXD_File *dmxd);
bool parse_dmx_file(DMX_File *dmx);
bool resolve_note(DMXD_File *dmxd, DMX_Instruction *instr, int *note_out);
bool visualizer_load_sequence(DMX_File *dmx, DMXD_File *dmxd);