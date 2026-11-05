#include "tetroid.h"
#include "matrix.h"
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

// Classic NES gravity rates for displayed levels 1 through 19, converted from frames at 60 Hz to milliseconds.
static const int GravityMs[] = {
    800, 717, 633, 550, 467, 383, 300, 217, 133, 100,
    83, 83, 83, 67, 67, 50, 50, 33, 17
};

// Private function declarations
static bool is_valid_tetroid(const Tetromino *t, const Matrix *playfield);

/**
 * Generate a random Tetromino
 */
bool tetromino_has_visible_cell(const Tetromino *piece, int first_visible_row) {
    if (!piece || !piece->matrix.cells || piece->matrix.rows <= 0 ||
        piece->matrix.cols <= 0) return false;

    for (int row = 0; row < piece->matrix.rows; row++) {
        if (piece->y_pos + row < first_visible_row) continue;
        for (int col = 0; col < piece->matrix.cols; col++) {
            if (piece->matrix.cells[row * piece->matrix.cols + col].active) {
                return true;
            }
        }
    }
    return false;
}

Tetromino generate_piece(PieceTemplate piece_template) {
    Matrix matrix = matrix_create_from_array(piece_template.rows, piece_template.cols, piece_template.cells);
    int x_pos = matrix.cells ? (10 - matrix.cols) / 2 : 0;
    return (Tetromino){
        .x_pos = x_pos,
        .y_pos = 0,
        .matrix_size = matrix.rows,
        .matrix = matrix
    };
}

Tetromino generate_random_piece(void) {
    size_t piece_count = sizeof(PIECES) / sizeof(PIECES[0]);
    size_t piece_index = (size_t)rand() % piece_count;
    return generate_piece(PIECES[piece_index]);
}

int get_level_gravity(int level) {
    const int level_count = (int)(sizeof(GravityMs) / sizeof(GravityMs[0]));
    if (level < 1) level = 1;
    if (level > level_count) level = level_count;
    return GravityMs[level - 1];
}

bool can_rotate(Tetromino *tActive, Matrix *playfield) {
    if (!is_valid_tetroid(tActive, playfield)) return false;
    Matrix rotated = matrix_rotated(&tActive->matrix, MATRIX_ROTATE_CW);
    if (!rotated.cells) return false;
    bool can_rotate = !matrix_overlap(&rotated, playfield, tActive->y_pos, tActive->x_pos, NULL, NULL);
    matrix_free(&rotated);
    return can_rotate;
}

bool try_rotate_with_wall_kick(Tetromino *tActive, const Matrix *playfield) {
    if (!is_valid_tetroid(tActive, playfield)) return false;

    Matrix rotated = matrix_rotated(&tActive->matrix, MATRIX_ROTATE_CW);
    if (!rotated.cells) return false;

    // Try the current position first, then kick away from a wall or obstacle.
    static const int horizontal_kicks[] = {0, -1, 1, -2, 2};
    for (size_t i = 0; i < sizeof(horizontal_kicks) / sizeof(horizontal_kicks[0]); i++) {
        int candidate_x = tActive->x_pos + horizontal_kicks[i];
        if (!matrix_overlap(&rotated, playfield, tActive->y_pos, candidate_x, NULL, NULL)) {
            matrix_free(&tActive->matrix);
            tActive->matrix = rotated;
            tActive->x_pos = candidate_x;
            tActive->matrix_size = rotated.rows;
            return true;
        }
    }

    matrix_free(&rotated);
    return false;
}

bool can_move_down(const Tetromino *tActive, const Matrix *playfield) {
    if(!is_valid_tetroid(tActive, playfield)) return false;
    if(matrix_overlap(&tActive->matrix, playfield, tActive->y_pos + 1, tActive->x_pos, NULL, NULL)) return false;
    return true;
}

bool can_move_left(const Tetromino *tActive, const Matrix *playfield) {
    if(!is_valid_tetroid(tActive, playfield)) return false;
    if(matrix_overlap(&tActive->matrix, playfield, tActive->y_pos, tActive->x_pos - 1, NULL, NULL)) return false;
    return true;
}

bool can_move_right(const Tetromino *tActive, const Matrix *playfield) {
    if (!is_valid_tetroid(tActive, playfield)) return false;
    if(matrix_overlap(&tActive->matrix, playfield, tActive->y_pos, tActive->x_pos + 1, NULL, NULL)) return false;
    return true;
}

int remove_completed_rows(Matrix *playfield) {
    if (!playfield || !playfield->cells ||
        playfield->rows <= 0 || playfield->cols <= 0) return 0;

    bool *rows_to_remove = calloc((size_t)playfield->rows, sizeof(*rows_to_remove));
    if (!rows_to_remove) return 0;

    for (int row = 0; row < playfield->rows; row++) {
        rows_to_remove[row] = !matrix_row_has_inactive_cells(playfield, row);
    }

    int rows_removed = matrix_collapse_rows(playfield, rows_to_remove);
    free(rows_to_remove);
    return rows_removed;
}

int calculate_score(int rows_removed, int level) {
    static const int base_scores[] = {0, 40, 100, 300, 1200};
    if (level < 1 || rows_removed < 0 || rows_removed > 4) return 0;

    int64_t score = (int64_t)base_scores[rows_removed] * level;
    return score > INT_MAX ? INT_MAX : (int)score;
}

void update_level(int lines_removed, int *lines, int *lines_to_level, int *level, int *gravity) {
    if (!lines || !lines_to_level || !level || !gravity || lines_removed < 0) return;

    *lines += lines_removed;
    int new_level = *lines / 10 + 1;
    if (new_level != *level) {
        *level = new_level;
        *gravity = get_level_gravity(*level);
    }
    *lines_to_level = 10 - (*lines % 10);
}

static bool is_valid_tetroid(const Tetromino *t, const Matrix *playfield) {
    return t != NULL
        && playfield != NULL
        && t->matrix.cells != NULL
        && t->matrix.cols > 0
        && t->matrix.rows > 0;
}
