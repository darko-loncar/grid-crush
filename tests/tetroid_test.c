#include "../tetroid.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static void test_line_clear_scoring(void) {
    assert(calculate_score(0, 1) == 0);
    assert(calculate_score(1, 1) == 40);
    assert(calculate_score(2, 1) == 100);
    assert(calculate_score(3, 1) == 300);
    assert(calculate_score(4, 1) == 1200);
    assert(calculate_score(4, 11) == 13200);
    assert(calculate_score(4, INT_MAX) == INT_MAX);
    assert(calculate_score(-1, 1) == 0);
    assert(calculate_score(5, 1) == 0);
    assert(calculate_score(1, 0) == 0);
}

static void test_level_progression(void) {
    int lines = 0;
    int lines_to_level = 10;
    int level = 1;
    int gravity = get_level_gravity(level);

    assert(gravity == 800);
    update_level(9, &lines, &lines_to_level, &level, &gravity);
    assert(lines == 9 && lines_to_level == 1 && level == 1 && gravity == 800);

    update_level(2, &lines, &lines_to_level, &level, &gravity);
    assert(lines == 11 && lines_to_level == 9 && level == 2 && gravity == 717);

    update_level(20, &lines, &lines_to_level, &level, &gravity);
    assert(lines == 31 && lines_to_level == 9 && level == 4 && gravity == 550);
}

static void test_generate_piece_from_template(void) {
    const MatrixCell cells[] = {
        {true, 7, 1, 2, 3}, {false, 0, 0, 0, 0}, {true, 8, 4, 5, 6},
        {false, 0, 0, 0, 0}, {true, 9, 7, 8, 9}, {false, 0, 0, 0, 0}
    };
    PieceTemplate piece_template = {2, 3, cells};
    Tetromino piece = generate_piece(piece_template);

    assert(piece.matrix.cells);
    assert(piece.matrix.rows == 2 && piece.matrix.cols == 3);
    assert(piece.x_pos == 3 && piece.y_pos == 0 && piece.matrix_size == 2);
    assert(piece.matrix.cells[0].active && piece.matrix.cells[0].value == 7);
    assert(piece.matrix.cells[2].active && piece.matrix.cells[2].value == 8);
    assert(piece.matrix.cells[4].active && piece.matrix.cells[4].value == 9);
    matrix_free(&piece.matrix);
}

static void test_classic_piece_set(void) {
    srand(12345);
    for (int i = 0; i < 100; i++) {
        Tetromino piece = generate_random_piece();
        assert(piece.matrix.cells);

        int occupied = 0;
        for (int cell = 0; cell < piece.matrix.rows * piece.matrix.cols; cell++) {
            occupied += piece.matrix.cells[cell].active;
        }
        assert(occupied == 4);
        matrix_free(&piece.matrix);
    }
}

static void test_gravity_curve(void) {
    assert(get_level_gravity(0) == 800);
    assert(get_level_gravity(1) == 800);
    assert(get_level_gravity(10) == 100);
    assert(get_level_gravity(19) == 17);
    assert(get_level_gravity(100) == 17);
}

int main(void) {
    test_line_clear_scoring();
    test_level_progression();
    test_generate_piece_from_template();
    test_classic_piece_set();
    test_gravity_curve();
    puts("tetroid tests passed");
    return 0;
}
