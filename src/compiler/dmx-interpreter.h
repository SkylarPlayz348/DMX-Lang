#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct DMX_File
{
    int bpm;
    int channel;
    char *controller;
    FILE *handler;
} DMX_File;