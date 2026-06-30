#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../src/midi.h"

int main()
{
    MIDI_File midi_file;
    midi_file.handler  = fopen("./manual_midi_file.midi", "wb+");
    if(!midi_file.handler)
    {
        printf("Failed to open midi file\n");
        return -1;
    }
    fwrite("MThd", sizeof(char) * 4, 1, midi_file.handler);
    midi_file.header.size = htonl(6);
    midi_file.header.format = ((uint16_t) 0);
    midi_file.header.ntrcks = ((uint16_t) 1);
    midi_file.header.division = ((uint16_t) 0x7FFF);
    fwrite(&midi_file.header, sizeof(MIDI_Header), 1, midi_file.handler);
    printf("Wrote Midi File!\n");
    fclose(midi_file.handler);
    return 0;
}