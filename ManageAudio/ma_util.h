
/** This structure describes a menu entry */

#ifndef __MA_UTIL__
#define __MA_UTIL__

typedef struct
{
    uint8_t brightness;
} t_persistent;

void read_from_persistent(t_persistent* persistent);
void write_to_persistent(t_persistent* persistent);

#endif

