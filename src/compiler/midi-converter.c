#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <midi.h>
#include <midi_helper.h>
#include <midi_constants.h>

bool write_midi_file(char *midi_file)
{
    /*
    Midi *m = malloc(sizeof(Midi));
    int delta = 0;
    FILE *f = fopen(midi_file, "wb+");
    if(!f){
        return false;
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

    // scene 1-7
    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_ON, CHANNEL_4);
    e = add_byte(e, 6);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, 0, e->event_string, e->event_string_len);
    free_event_string(e);


    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_OFF, CHANNEL_4);
    e = add_byte(e, 6);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, delta+=384, e->event_string, e->event_string_len);
    free_event_string(e);

    // scene 5-7
    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_ON, CHANNEL_4);
    e = add_byte(e, 38);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, 0, e->event_string, e->event_string_len);
    free_event_string(e);

    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_OFF, CHANNEL_4);
    e = add_byte(e, 38);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, delta+=384, e->event_string, e->event_string_len);
    free_event_string(e);

    // blackout
    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_ON, CHANNEL_4);
    e = add_byte(e, 126);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, 0, e->event_string, e->event_string_len);
    free_event_string(e);

    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_OFF, CHANNEL_4);
    e = add_byte(e, 126);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, delta+=384, e->event_string, e->event_string_len);
    free_event_string(e);

    // scene 15-1
    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_ON, CHANNEL_4);
    e = add_byte(e, 112);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, 0, e->event_string, e->event_string_len);
    free_event_string(e);

    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_OFF, CHANNEL_4);
    e = add_byte(e, 112);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, delta+=384, e->event_string, e->event_string_len);
    free_event_string(e);

    // blackout
    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_ON, CHANNEL_4);
    e = add_byte(e, 126);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, 0, e->event_string, e->event_string_len);
    free_event_string(e);

    e = new_event_string(e);
    e = add_voice_message(e, VOICE_NOTE_OFF, CHANNEL_4);
    e = add_byte(e, 126);
    e = add_byte(e, VELOCITY_MAX);
    track_add_event_full(track, delta+=384, e->event_string, e->event_string_len);
    free_event_string(e);

    // End Midi Sequence
    e = new_event_string(e);
	e = add_meta_message(e, META_END);
	e = add_string(e, "", 0);
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
    free_event_string(e);
    */

    //create a new midi
	struct Midi* m = malloc(sizeof(struct Midi));
	new_midi(m);
	//set it to mode 0, with 1 track, and 384 ticks per quarternote
	midi_add_header(m, 0, 1, 384);

	//add a track
	struct MidiTrackChunk* track = midi_add_track(m);

	struct EventString* e = malloc(sizeof(struct EventString));

	//add a meta event which specifies the track name as "Trumpet"
	e = new_event_string(e);
	e = add_meta_message(e, META_TRACK_NAME);
	e = add_string(e, "Trumpet", strlen("Trumpet"));
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

	//Add a voice event to change the instrument to the trumpet program
	//it's cheaper to reuse the same EventString (although not much)
	e = new_event_string(e);
	e = add_voice_message(e, VOICE_PROGRAM_CHANGE, CHANNEL_0);
	e = add_byte(e, INSTRUMENT_TRUMPET);
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

	//start playing a C4 at mezzo-forte
	e = new_event_string(e);
	e = add_voice_message(e, VOICE_NOTE_ON, CHANNEL_0);
	e = add_byte(e, NOTE_C4);
	e = add_byte(e, VELOCITY_MEZZOFORTE);
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

	//after 3072 ticks (2 whole notes) stop playing the C4
	e = new_event_string(e);
	e = add_voice_message(e, VOICE_NOTE_OFF, CHANNEL_0);
	e = add_byte(e, NOTE_C4);
	e = add_byte(e, VELOCITY_MEZZOFORTE);
	track_add_event_full(track, 3072, e->event_string, e->event_string_len);
	free_event_string(e);

	//end the track
	e = new_event_string(e);
	e = add_meta_message(e, META_END);
	e = add_string(e, "", 0);
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

    FILE *f = fopen(midi_file, "wb+");
    write_midi(m, f);
    fclose(f);
    free(m);
    free(e);
    return true;
}