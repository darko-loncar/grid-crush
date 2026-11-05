#ifndef UI_H
#define UI_H

#include <stdbool.h>

#define UI_PANEL_X 450.0f
#define UI_PREVIEW_X 625.0f
#define UI_PREVIEW_NEXT_Y 120
#define UI_PREVIEW_SMALL_2_Y 112
#define UI_PREVIEW_SMALL_3_Y 192

#define UI_BUTTON_W 200.0f
#define UI_BUTTON_H 46.0f
#define UI_NEW_GAME_Y 470.0f
#define UI_QUIT_Y 530.0f


typedef struct {
    bool menu;
    bool paused;
    bool game_over;

    bool new_game_hovered;
    bool quit_hovered;

    int score;
    int lines;
    int level;
} UiView;

void ui_draw(const UiView *view);

#endif
