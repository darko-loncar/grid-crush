#ifndef INPUT_H
#define INPUT_H

#include "tetroid.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

typedef enum {
    GAME_MENU,
    GAME_PLAYING,
    GAME_PAUSED,
    GAME_OVER
} GameState;

typedef struct {
    bool *running;
    GameState *game_state;


    Tetromino *active_piece;
    Matrix *playfield;

    int *gravity;
    int *level;
    Uint64 *last_tick;
    bool (*start_new_game)(void);
    void (*lock_active_piece)(void);
} InputContext;

/** Handle one SDL event using references to the current game state. */
void input_handle_event(const SDL_Event *event, InputContext *context);


/** Test whether the tracked pointer is inside a rectangle. */
bool input_pointer_in_rect(float rect_x, float rect_y, float w, float h);

#endif
