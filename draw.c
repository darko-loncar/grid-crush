#include "draw.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

SDL_Renderer *renderer = NULL;  // The single definition
TTF_Font *font;

bool initDraw(SDL_Window *window){
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_Quit();
        return NULL;
    }

    if (!TTF_Init()) {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
        return NULL;
    }

    font = TTF_OpenFont("assets/Orbitron.ttf", 24.0f);

    if (!font) {
        SDL_Log("TTF_OpenFont failed: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return NULL;
    }
    return true;
}

void destroyDraw(void) {
    if (font != NULL) {
        TTF_CloseFont(font);
        font = NULL;
    }
    TTF_Quit();

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
}

SDL_Texture *createLightGrainTexture(int width, int height) {
    if (!renderer || width <= 0 || height <= 0) return NULL;

    if (width > INT_MAX / 4 || (size_t)width > SIZE_MAX / (size_t)height) return NULL;
    size_t pixel_count = (size_t)width * (size_t)height;
    if (pixel_count > SIZE_MAX / 4) return NULL;

    Uint8 *pixels = malloc(pixel_count * 4);
    if (!pixels) return NULL;

    // Local PRNG keeps the texture reproducible without changing rand() state.
    uint32_t state = 0xA341316Cu;
    for (size_t i = 0; i < pixel_count; i++) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;

        Uint8 shade = (Uint8)(218 + (state % 38));
        pixels[i * 4 + 0] = shade;
        pixels[i * 4 + 1] = shade;
        pixels[i * 4 + 2] = shade;
        pixels[i * 4 + 3] = 24;
    }

    SDL_Texture *texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height
    );
    if (!texture || !SDL_UpdateTexture(texture, NULL, pixels, width * 4) ||
        !SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND)) {
        if (texture) SDL_DestroyTexture(texture);
        free(pixels);
        return NULL;
    }

    free(pixels);
    return texture;
}

void drawBackgroundGradient(int width, int height) {
    const int top_r = 37, top_g = 47, top_b = 73;
    const int bot_r = 21, bot_g = 26, bot_b = 38;

    for (int y = 0; y < height; y++) {
        float t = (float)y / (float)(height - 1);

        Uint8 r = (Uint8)(top_r + (bot_r - top_r) * t);
        Uint8 g = (Uint8)(top_g + (bot_g - top_g) * t);
        Uint8 b = (Uint8)(top_b + (bot_b - top_b) * t);

        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_FRect line = {.x = 0, .y = (float)y, .w = (float)width, .h = 1.0f};
        SDL_RenderFillRect(renderer, &line);
    }
}

void drawText(const char *text, float x, float y) {
    SDL_Color color = { COLOR_WHITE, 255 };
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, 0, color);
    if (!surface) {
        SDL_Log("Text rendering failed: %s", SDL_GetError());
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_FRect dst = { x, y, (float)surface->w, (float)surface->h };
        SDL_RenderTexture(renderer, texture, NULL, &dst);
        SDL_DestroyTexture(texture);
    }
    SDL_DestroySurface(surface);
}


/**
 * Draws a cell using its color and the requested square pixel size.
 * Smaller sizes use proportionally smaller highlights.
 */
void drawCell(const MatrixCell *cell, int x_offset, int y_offset, int pixel_size) {
    if (!cell || pixel_size <= 0) return;
    SDL_FRect block = {x_offset, y_offset, pixel_size, pixel_size};
    SDL_SetRenderDrawColor(renderer, cell->r, cell->g, cell->b, 255);
    SDL_RenderFillRect(renderer, &block);
    SDL_SetRenderDrawColor(renderer, (uint8_t)(cell->r * 0.75f), (uint8_t)(cell->g * 0.75f), (uint8_t)(cell->b * 0.75f), 255);
    SDL_RenderRect(renderer, &block);

    int inset = pixel_size >= 12 ? 3 : 1;
    int highlight_height = pixel_size >= 12 ? 2 : 1;
    SDL_FRect highlight = {
        x_offset + inset, y_offset + inset,
        pixel_size - inset * 2, highlight_height
    };
    SDL_SetRenderDrawColor(
        renderer,
        (Uint8)(cell->r + (255 - cell->r) * 0.25f),
        (Uint8)(cell->g + (255 - cell->g) * 0.25f),
        (Uint8)(cell->b + (255 - cell->b) * 0.25f),
        255
    );
    SDL_RenderFillRect(renderer, &highlight);
}

void drawMatrix(const Matrix *m, int x_offset, int y_offset) {

    const MatrixCell *cell;

    // Calculate total cell number
    int cell_count = m->cols * m->rows;

    // Loop through all cells
    for(int cell_idx = 0; cell_idx < cell_count; cell_idx++) {
        cell = &m->cells[cell_idx];

        if(cell->active == 0) continue;

        // Calculate cell col and row
        int cell_col = (cell_idx % m->cols); // cell col relative to active matrix
        int cell_row = (cell_idx / m->cols); // cell row relative to active matrix

        // Calculate cell screen coords
        int x = cell_col * BLOCK_SIZE + x_offset;
        int y = cell_row * BLOCK_SIZE + y_offset;

        // Draw cell
        drawCell(cell, x, y, BLOCK_SIZE);
    }
}

void drawMatrixSmall(const Matrix *m, int x_offset, int y_offset) {
    if (!m || !m->cells || m->rows <= 0 || m->cols <= 0) return;

    for (int row = 0; row < m->rows; row++) {
        for (int col = 0; col < m->cols; col++) {
            const MatrixCell *cell = &m->cells[row * m->cols + col];
            if (!cell->active) continue;
            drawCell(cell,
                     x_offset + col * BLOCK_SIZE_SMALL,
                     y_offset + row * BLOCK_SIZE_SMALL,
                     BLOCK_SIZE_SMALL);
        }
    }
}

void drawGhostCell(const MatrixCell *cell, int x_offset, int y_offset) {
    if (!cell || !cell->active) return;

    SDL_FRect ghost = {
        (float)x_offset + 3.0f,
        (float)y_offset + 3.0f,
        BLOCK_SIZE - 6.0f,
        BLOCK_SIZE - 6.0f
    };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 85, 140, 155, 70);
    SDL_RenderRect(renderer, &ghost);
}

void drawGhostPiece(const Matrix *piece, int x_offset, int y_offset) {
    if (!piece || !piece->cells || piece->rows <= 0 || piece->cols <= 0) return;

    for (int row = 0; row < piece->rows; row++) {
        for (int col = 0; col < piece->cols; col++) {
            const MatrixCell *cell = &piece->cells[row * piece->cols + col];
            if (!cell->active) continue;
            drawGhostCell(cell, x_offset + col * BLOCK_SIZE,
                          y_offset + row * BLOCK_SIZE);
        }
    }
}


void drawPlayfield(const Matrix *playfield, int x_offset, int y_offset) {
    SDL_FRect frame;

    int width = playfield->cols * BLOCK_SIZE;
    int height = playfield->rows * BLOCK_SIZE;

    // Draw outer playfield frame
    SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);

    frame = (SDL_FRect){ x_offset - 1, y_offset - 1, width + 2, height + 2 };
    SDL_RenderFillRect(renderer, &frame);

    frame = (SDL_FRect){ x_offset - 2, y_offset - 2, width + 4, height + 4 };
    SDL_RenderRect(renderer, &frame);

    SDL_SetRenderDrawColor(renderer, 52, 72, 96, 255);

    frame = (SDL_FRect){ x_offset - 3, y_offset - 3, width + 6, height + 6 };
    SDL_RenderRect(renderer, &frame);

    frame = (SDL_FRect){ x_offset - 4, y_offset - 4, width + 8, height + 8 };
    SDL_RenderRect(renderer, &frame);

    // Draw background grid
    SDL_SetRenderDrawColor(renderer, 20,20,20, 255);

    for (int col = 0; col <= playfield->cols; col++) {
        float x = (float)(x_offset + col * BLOCK_SIZE);
        SDL_RenderLine(renderer, x, (float)y_offset, x, (float)(y_offset + height));
    }

    for (int row = 0; row <= playfield->rows; row++) {
        float y = (float)(y_offset + row * BLOCK_SIZE);
        SDL_RenderLine(renderer, (float)x_offset, y, (float)(x_offset + width), y);
    }
}
