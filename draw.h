#ifndef DRAW_H
#define DRAW_H

#include "matrix.h"
#include "color.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#define BLOCK_SIZE 40
#define BLOCK_SIZE_SMALL 15

extern SDL_Renderer *renderer;
extern TTF_Font *font;

bool initDraw(SDL_Window *window);
void destroyDraw(void);
void drawBackgroundGradient(int width, int height);
SDL_Texture *createLightGrainTexture(int width, int height);
void drawText(const char *text, float x, float y);
void drawMatrix(const Matrix *m, int x_offset, int y_offset);
void drawMatrixSmall(const Matrix *m, int x_offset, int y_offset);
void drawGhostPiece(const Matrix *piece, int x_offset, int y_offset);
void drawGhostCell(const MatrixCell *cell, int x_offset, int y_offset);

void drawPlayfield(const Matrix *playfield, int x_offset, int y_offset);
void drawCell(const MatrixCell *cell, int x_offset, int y_offset, int pixel_size);

#endif
