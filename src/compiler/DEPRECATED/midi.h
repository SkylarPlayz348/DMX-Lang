/*
    midi.h was a file where all our custom MIDI classes and stuff go for our compiler
*/
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/**********From https://github.com/pvrs12/midi_library/blob/master/src/midi.c**********/
#define LITTLE_ENDIAN 0x41424344UL
#define BIG_ENDIAN    0x44434241UL
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"
#define ENDIAN_ORDER  ('ABCD')

#if ENDIAN_ORDER==LITTLE_ENDIAN
#define htons(A) 	((((uint16_t)(A) & 0xff00) >> 8) | \
					(((uint16_t)(A) & 0x00ff) << 8))

#define htonl(A) 	((((uint32_t)(A) & 0xff000000) >> 24) | \
					(((uint32_t)(A) & 0x00ff0000) >> 8)  | \
					(((uint32_t)(A) & 0x0000ff00) << 8)  | \
					(((uint32_t)(A) & 0x000000ff) << 24))

#define ntohs  htons
#define ntohl  htonl
#elif ENDIAN_ORDER==BIG_ENDIAN
	#define htons(A) (A)
	#define htonl(A) (A)
	#define ntohs(A) (A)
	#define ntohl(A) (A)

#else
	#error "Cannot determine endianness. Good luck."
#endif //ENDIAN_ORDER
/*******************************************************************************/

typedef

typedef struct MIDI_Header
{
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