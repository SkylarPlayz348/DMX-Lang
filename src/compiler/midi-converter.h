#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <midi.h>
#include <midi_helper.h>
#include <midi_constants.h>

#include "dmx-interpreter.h"

#define TICKS_PER_QUARTER 480
#define NOTE_PRESS_TICKS TICKS_PER_QUARTER
#define DEFAULT_BPM 120

bool write_midi_file(char *midi_file, DMX_File *dmx, const DMXD_File *dmxd);