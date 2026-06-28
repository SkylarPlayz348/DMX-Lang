/*
    midi.h is a file where all our custom MIDI classes and stuff go for our compiler
*/
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct MIDI_Header
{
    char chunk_id[4];
    uint32_t size;
    uint16_t format;
    uint16_t ntrcks;
    uint16_t division;
} MIDI_Header;

typedef struct MIDI_Mtrk_Event
{
    uint delta;
    uint type;
    uint32_t size;
    void *data;
} MIDI_Mtrk_Event;

typedef struct MIDI_Mtrk
{
    char chunk_id[4];
    uint16_t length;
    MIDI_Mtrk_Event* events;
} MIDI_Mtrk;

typedef struct MIDI_File
{
    FILE *handler;
    MIDI_Header header;
    MIDI_Mtrk *tracks;
} MIDI_File;