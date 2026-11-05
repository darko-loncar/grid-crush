#include "ui.h"
#include "draw.h"
#include <stdio.h>

static void drawButton(const char *label, float x, float y, bool hovered);


void ui_draw(const UiView *view) {
    if (!view) return;

    drawText("TETROID", UI_PANEL_X, 30.0f);

    if (view->menu) {
        drawText("Welcome!", UI_PANEL_X, 120.0f);
        drawText("Start a new game", UI_PANEL_X, 175.0f);
        drawText("Use arrow keys to move", UI_PANEL_X, 225.0f);
        drawText("P to pause", UI_PANEL_X, 265.0f);
    } else {
        drawText("Next piece:", UI_PANEL_X, 80.0f);
        drawText("2nd", UI_PREVIEW_X, 80.0f);
        drawText("3rd", UI_PREVIEW_X, 160.0f);

        char score_text[32];
        char lines_text[32];
        char level_text[32];
        snprintf(score_text, sizeof(score_text), "Score: %d", view->score);
        snprintf(lines_text, sizeof(lines_text), "Lines: %d", view->lines);
        snprintf(level_text, sizeof(level_text), "Level: %d", view->level);
        drawText(score_text, UI_PANEL_X, 230.0f);
        drawText(lines_text, UI_PANEL_X, 280.0f);
        drawText(level_text, UI_PANEL_X, 330.0f);

        if (view->paused) drawText("PAUSED", UI_PANEL_X, 390.0f);
        if (view->game_over) drawText("GAME OVER", UI_PANEL_X, 390.0f);
    }


    drawButton("New Game", UI_PANEL_X, UI_NEW_GAME_Y,
               view->new_game_hovered);
    drawButton("Quit", UI_PANEL_X, UI_QUIT_Y, view->quit_hovered);
}

static void drawButton(const char *label, float x, float y, bool hovered) {
    SDL_FRect button = {x, y, UI_BUTTON_W, UI_BUTTON_H};
    if (hovered) {
        SDL_SetRenderDrawColor(renderer, 72, 130, 190, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 47, 76, 112, 255);
    }
    SDL_RenderFillRect(renderer, &button);
    SDL_SetRenderDrawColor(renderer, 145, 180, 220, 255);
    SDL_RenderRect(renderer, &button);
    drawText(label, x + 18.0f, y + 9.0f);
}


