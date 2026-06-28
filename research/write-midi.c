#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../src/midi.h"

size_t lendian_fwrite(const void *ptr, size_t size, size_t nmemb,
                     FILE *stream)
{
    if (size != 4)
    {
        /* Warn and exit */
    }

    int x = 1;

    if ( *((char*)&x) == 1)
    {
        /* Little endian machine, use fwrite directly */
        return fwrite(ptr, size, nmemb, stream);
    }
    else
    {
        /* Big endian machine, pre-process first */

        unsigned char *buffer = (unsigned char*) ptr;

        for (int i=0; i<nmemb; i++)
        {           
            unsigned char a = buffer[4*i];
            unsigned char b = buffer[4*i + 1];

            buffer[4*i] = buffer[4*i + 3];
            buffer[4*i + 1] = buffer[4*i + 2];
            buffer[4*i + 2] = b;
            buffer[4*i + 3] = a;
        }

        return fwrite(ptr, size, nmemb, stream);
    }  
}

int main()
{
    MIDI_File midi_file;
    midi_file.handler  = fopen("./manual_midi_file.midi", "wb+");

    strncpy(midi_file.header.chunk_id, "MThd", 4);
    midi_file.header.size = 6;
    midi_file.header.format = 0;
    midi_file.header.ntrcks = 1;
    midi_file.header.division = 0x7FFF;


    lendian_fwrite(&midi_file.header, sizeof(MIDI_Header), 1, midi_file.handler);
    fclose(midi_file.handler);
}