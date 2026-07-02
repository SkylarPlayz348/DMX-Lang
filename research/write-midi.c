#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <midi.h>
#include <midi_helper.h>
#include <midi_constants.h>

/*
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
    midi_file.header.division = ((uint16_t) 0x0180);
    fwrite(&midi_file.header, sizeof(MIDI_Header)-2, 1, midi_file.handler);
    fwrite("MTrk", sizeof(char)*4, 1, midi_file.handler);
    printf("Wrote Midi File!\n");
    fclose(midi_file.handler);
    return 0;
} */

// Setup like my controller would need it
int main()
{
    Midi *m = malloc(sizeof(Midi));
    FILE *f = fopen("manual_midi_file.midi", "wb+");
    if(!f){
        printf("Failed to open manual_midi_file.midi");
        return -1;
    }
    new_midi(m);
    midi_add_header(m, 0, 1, 384);

    MidiTrackChunk *track = midi_add_track(m);
    struct EventString *e = malloc(sizeof(EventString));

    e = new_event_string(e);
    e = add_meta_message(e, META_TRACK_NAME);
    e = add_string(e, "Obey 70 Channel 4", strlen("Obey 70 Channel 4"));
    track_add_event_full(track, 0, e->event_string, e->event_string_len);
    free_event_string(e);

    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_ON, CHANNEL_4);
    e = add_byte(e, 120);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, 0, e->event_string, e->event_string_len);
    free_event_string(e);

    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_OFF, CHANNEL_4);
    e = add_byte(e, 120);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, 384, e->event_string, e->event_string_len);
    free_event_string(e);

    e = new_event_string(e);
	e = add_meta_message(e, META_END);
	e = add_string(e, "", 0);
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
    free_event_string(e);

    write_midi(m, f);
    fclose(f);
    free(m);
    free(e);
    printf("Wrote Midi File!\n");
    return 0;
}