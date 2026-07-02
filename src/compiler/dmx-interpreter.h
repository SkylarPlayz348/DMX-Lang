#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct DMX_File
{
    int bpm;
    int channel;
    char *controller;
    FILE *handler;
} DMX_File;

bool check_dmx_file(DMX_File *dmx);
bool parse_dmx_file(DMX_File *dmx);