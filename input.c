#include "input.h"
#include "ui.h"

#define GRAVITY_SOFT_DROP 50

static float mouse_x;
static float mouse_y;

bool input_pointer_in_rect(float rect_x, float rect_y, float w, float h) {
    return mouse_x >= rect_x && mouse_x < rect_x + w &&
           mouse_y >= rect_y && mouse_y < rect_y + h;
}


void input_handle_event(const SDL_Event *event, InputContext *context) {
    if (!event || !context || !context->running || !context->game_state) return;

    if (event->type == SDL_EVENT_QUIT) {
        *context->running = false;
        return;
    }

    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        mouse_x = event->motion.x;
        mouse_y = event->motion.y;
        return;
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && event->button.button == SDL_BUTTON_LEFT) {
        mouse_x = event->button.x;
        mouse_y = event->button.y;
        if (context->start_new_game &&
            input_pointer_in_rect(UI_PANEL_X, UI_NEW_GAME_Y, UI_BUTTON_W, UI_BUTTON_H)) {
            context->start_new_game();
        } else if (input_pointer_in_rect(UI_PANEL_X, UI_QUIT_Y, UI_BUTTON_W, UI_BUTTON_H)) {
            *context->running = false;
        }
        return;
    }

    if (event->type == SDL_EVENT_KEY_DOWN && !event->key.repeat) {
        if (event->key.key == SDLK_ESCAPE) {
            *context->running = false;
            return;
        }

        if (*context->game_state == GAME_MENU || *context->game_state == GAME_OVER) {
            if (context->start_new_game &&
                (event->key.key == SDLK_RETURN || event->key.key == SDLK_SPACE)) {
                context->start_new_game();
            }
            return;
        }


        if (event->key.key == SDLK_P) {
            *context->game_state = *context->game_state == GAME_PAUSED ? GAME_PLAYING : GAME_PAUSED;
            if (context->last_tick) *context->last_tick = SDL_GetTicks();
            return;
        }

        if (*context->game_state != GAME_PLAYING || !context->active_piece ||
            !context->playfield) return;

        switch (event->key.key) {
            case SDLK_UP:
                try_rotate_with_wall_kick(context->active_piece, context->playfield);
                break;
            case SDLK_LEFT:
                if (can_move_left(context->active_piece, context->playfield)) {
                    context->active_piece->x_pos--;
                }
                break;
            case SDLK_RIGHT:
                if (can_move_right(context->active_piece, context->playfield)) {
                    context->active_piece->x_pos++;
                }
                break;
            case SDLK_DOWN:
                if (context->gravity) *context->gravity = GRAVITY_SOFT_DROP;
                break;
            case SDLK_SPACE:
                while (can_move_down(context->active_piece, context->playfield)) {
                    context->active_piece->y_pos++;
                }
                if (context->lock_active_piece) context->lock_active_piece();
                break;
            default:
                break;
        }
    }

    if (event->type == SDL_EVENT_KEY_UP && event->key.key == SDLK_DOWN &&
        *context->game_state == GAME_PLAYING && context->gravity && context->level) {
        *context->gravity = get_level_gravity(*context->level);
    }
}
