#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <midi.h>
#include <midi_helper.h>
#include <midi_constants.h>

#include "midi-converter.h"

uint32_t seconds_to_ticks(int seconds, int bpm)
{
    if(seconds <= 0)
        return 0;
    if(bpm <= 0)
        bpm = DEFAULT_BPM;
    return (uint32_t)((double)seconds * bpm / 60.0 * TICKS_PER_QUARTER);
}

bool write_midi_file(char *midi_file, DMX_File *dmx, const DMXD_File *dmxd)
{

}