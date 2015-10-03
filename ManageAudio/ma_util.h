
/** This structure describes a menu entry */

#ifndef __MA_UTIL__
#define __MA_UTIL__

#include "stdbool.h"

typedef struct
{
    uint8_t brightness;
    uint8_t audio_source;
} t_persistent;

#define SOURCE_MAX    4

/* EEPROM */
void read_from_persistent(t_persistent* persistent);
void write_to_persistent(t_persistent* persistent);

/* Source selection */
void source_selectA(uint8_t source, bool* outputs);

#endif

