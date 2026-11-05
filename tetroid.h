#ifndef TETROMINO_H
#define TETROMINO_H

#include "matrix.h"
#include "color.h"

// Tetroid piece
typedef struct {
    int x_pos;
    int y_pos;
    int matrix_size;
    Matrix matrix;
} Tetromino;

// Tetroid templates
typedef struct {
    int rows;
    int cols;
    const MatrixCell *cells;
} PieceTemplate;

static const MatrixCell T_CELLS[9] = {
    {0, 0, COLOR_T}, {1, 0, COLOR_T}, {0, 0, COLOR_T},
    {1, 0, COLOR_T}, {1, 0, COLOR_T}, {1, 0, COLOR_T},
    {0, 0, COLOR_T}, {0, 0, COLOR_T}, {0, 0, COLOR_T}
};

static const MatrixCell J_CELLS[9] = {
    {1, 0, COLOR_J}, {0, 0, COLOR_J}, {0, 0, COLOR_J},
    {1, 0, COLOR_J}, {1, 0, COLOR_J}, {1, 0, COLOR_J},
    {0, 0, COLOR_J}, {0, 0, COLOR_J}, {0, 0, COLOR_J}
};

static const MatrixCell L_CELLS[9] = {
    {0, 0, COLOR_L}, {0, 0, COLOR_L}, {1, 0, COLOR_L},
    {1, 0, COLOR_L}, {1, 0, COLOR_L}, {1, 0, COLOR_L},
    {0, 0, COLOR_L}, {0, 0, COLOR_L}, {0, 0, COLOR_L}
};

static const MatrixCell O_CELLS[4] = {
    {1, 0, COLOR_O}, {1, 0, COLOR_O},
    {1, 0, COLOR_O}, {1, 0, COLOR_O}
};

static const MatrixCell S_CELLS[9] = {
    {0, 0, COLOR_S}, {1, 0, COLOR_S}, {1, 0, COLOR_S},
    {1, 0, COLOR_S}, {1, 0, COLOR_S}, {0, 0, COLOR_S},
    {0, 0, COLOR_S}, {0, 0, COLOR_S}, {0, 0, COLOR_S}
};

static const MatrixCell Z_CELLS[9] = {
    {1, 0, COLOR_Z}, {1, 0, COLOR_Z}, {0, 0, COLOR_Z},
    {0, 0, COLOR_Z}, {1, 0, COLOR_Z}, {1, 0, COLOR_Z},
    {0, 0, COLOR_Z}, {0, 0, COLOR_Z}, {0, 0, COLOR_Z}
};

static const MatrixCell I_CELLS[16] = {
    {0, 0, COLOR_I}, {0, 0, COLOR_I}, {0, 0, COLOR_I}, {0, 0, COLOR_I},
    {1, 0, COLOR_I}, {1, 0, COLOR_I}, {1, 0, COLOR_I}, {1, 0, COLOR_I},
    {0, 0, COLOR_I}, {0, 0, COLOR_I}, {0, 0, COLOR_I}, {0, 0, COLOR_I},
    {0, 0, COLOR_I}, {0, 0, COLOR_I}, {0, 0, COLOR_I}, {0, 0, COLOR_I}
};

static const PieceTemplate PIECES[] = {
    {3, 3, T_CELLS},
    {3, 3, J_CELLS},
    {3, 3, L_CELLS},
    {2, 2, O_CELLS},
    {3, 3, S_CELLS},
    {3, 3, Z_CELLS},
    {4, 4, I_CELLS}
};

Tetromino generate_piece(PieceTemplate piece_template);
Tetromino generate_random_piece(void);
bool tetromino_has_visible_cell(const Tetromino *piece, int first_visible_row);
int get_level_gravity(int level);
bool can_rotate(Tetromino *active_piece, Matrix *playfield);
bool try_rotate_with_wall_kick(Tetromino *active_piece, const Matrix *playfield);
bool can_move_down(const Tetromino *active_piece, const Matrix *playfield);
bool can_move_left(const Tetromino *active_piece, const Matrix *playfield);
bool can_move_right(const Tetromino *active_piece, const Matrix *playfield);
int remove_completed_rows(Matrix *playfield);
int calculate_score(int rows_removed, int level);
void update_level(int lines_removed, int *lines, int *lines_to_level, int *level, int *gravity);
#endif // TETROMINO_H
