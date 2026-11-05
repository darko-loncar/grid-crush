#ifndef SOUND_H
#define SOUND_H

#include <stdbool.h>

bool sound_init(void);
void sound_play_piece_lock(void);
void sound_shutdown(void);

#endif
