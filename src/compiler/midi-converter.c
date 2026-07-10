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

void emit_trigger(MidiTrackChunk *track, EventString *e, int channel, int note, uint32_t lead_ticks)
{
    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_ON, (uint8_t)channel);
    e = add_byte(e, (uint8_t)note);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, lead_ticks, e->event_string, e->event_string_len);
    free_event_string(e);

    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_OFF, (uint8_t)channel);
    e = add_byte(e, (uint8_t)note);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, NOTE_PRESS_TICKS, e->event_string, e->event_string_len);
    free_event_string(e);
}

bool write_midi_file(char *midi_file, DMX_File *dmx, const DMXD_File *dmxd)
{
    int bpm = dmx->bpm > 0 ? dmx->bpm : DEFAULT_BPM;

    printf("BPM Set\n");

    Midi *m = malloc(sizeof(Midi));
    if(!m)
        return false;
    new_midi(m);
    midi_add_header(m, 0, 1, TICKS_PER_QUARTER);

    printf("Generated MIDI Header\n");

    MidiTrackChunk *track = midi_add_track(m);
    printf("Generated MIDI Track\n");

    EventString *e = malloc(sizeof(EventString));
    if(!e)
    {
        free_midi(m);
        free(m);
        return false;
    }

    printf("Populating MIDI Track\n");
    char track_name[128];
    snprintf(track_name, sizeof(track_name), "%s Channel %d", dmx->controller[0] ? dmx->controller : "Unamed", dmx->channel);
    e = new_event_string(e);
    e = add_meta_message(e, META_TRACK_NAME);
    e = add_string(e, track_name, strlen(track_name));
    track_add_event_full(track, 0, e->event_string, e->event_string_len);
    free_event_string(e);

    printf("Set Track Name\n");
    uint32_t us_per_quarter = 60000000u / (uint32_t)bpm;
    uint8_t tempo[3] = {
        (uint8_t)((us_per_quarter >> 16) & 0xFF),
        (uint8_t)((us_per_quarter >> 8) & 0xFF),
        (uint8_t)(us_per_quarter & 0xFF)
    };
    e = new_event_string(e);
    e = add_meta_message(e, META_SET_TEMPO);
    e = add_buffer(e, tempo, 3);
    track_add_event_full(track, 0, e->event_string, e->event_string_len);
    free_event_string(e);
    printf("Set Track Tempo\n");

    long clock = 0;
    long scheduled = 0;
    for(int i = 0; i < dmx->instruction_count; i++)
    {
        const DMX_Instruction *instr = &dmx->instructions[i];

        if(instr->kind == INSTR_DELAY)
        {
            scheduled += seconds_to_ticks(instr->seconds, bpm);
            continue;
        }

        int note = 0;
        if(!resolve_note(dmxd, instr, &note))
        {
            printf("Warning: no definition for '%s' (skipped)\n", instr->command);
            continue;
        }

        long on = scheduled > clock ? scheduled : clock;
        emit_trigger(track, e, dmx->channel, note, (uint32_t)(on - clock));
        clock = on + NOTE_PRESS_TICKS;
        scheduled = on;
    }

    long end_tick = scheduled > clock ? scheduled : clock;
    e = new_event_string(e);
    e = add_meta_message(e, META_END);
    e = add_string(e, "", 0);
    track_add_event_full(track, (uint32_t)(end_tick - clock), e->event_string, e->event_string_len);
    free_event_string(e);

    FILE *f = fopen(midi_file, "wb+");
    if(!f)
    {
        free_midi(m);
        free(m);
        free(e);
        return false;
    }
    write_midi(m, f);
    fclose(f);

    free_midi(m);
    free(m);
    free(e);
    return true;
}