#include "../matrix.h"
#include <assert.h>
#include <stdio.h>

static bool cell_has_value(const MatrixCell *cell, void *context) {
    return cell && context && cell->value == *(const int *)context;
}

static void test_create_access_and_setters(void) {
    Matrix m = matrix_create_empty(2, 3);
    assert(m.cells != NULL);
    assert(m.rows == 2 && m.cols == 3);

    MatrixCell *cell = matrix_get_cell(&m, 1, 2);
    assert(cell != NULL && !cell->active);
    matrix_set_cell_value(&m, 1, 2, 42);
    matrix_set_cell_color(&m, 1, 2, 10, 20, 30);
    cell->active = true;
    assert(cell->value == 42);
    assert(cell->r == 10 && cell->g == 20 && cell->b == 30);
    assert(matrix_get_cell(&m, -1, 0) == NULL);
    assert(matrix_get_cell(&m, 0, 3) == NULL);
    assert(matrix_get_row_first_cell(&m, 1) == &m.cells[3]);
    assert(matrix_get_row_first_cell(&m, 2) == NULL);

    assert(matrix_row_has_inactive_cells(&m, 1));
    m.cells[3].active = true;
    m.cells[4].active = true;
    assert(!matrix_row_has_inactive_cells(&m, 1));
    assert(matrix_row_has_inactive_cells(&m, -1));
    matrix_free(&m);
    assert(m.cells == NULL && m.rows == 0 && m.cols == 0);
}

static void test_create_from_file(void) {
    const char *path = "tests/.matrix_test_input.txt";
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    fputs("2 2\n10\n01\n", file);
    assert(fclose(file) == 0);

    Matrix m = matrix_create_from_file(path);
    assert(m.cells != NULL && m.rows == 2 && m.cols == 2);
    assert(m.cells[0].active && m.cells[0].value == 1);
    assert(m.cells[0].r == 40 && m.cells[0].g == 80 && m.cells[0].b == 255);
    assert(m.cells[1].active && m.cells[1].value == 0);
    assert(m.cells[3].active && m.cells[3].value == 1);
    matrix_free(&m);
    assert(remove(path) == 0);

    file = fopen(path, "w");
    assert(file != NULL);
    fputs("1 2\n1 7 10 20 30\n", file); // Missing the second cell.
    assert(fclose(file) == 0);
    m = matrix_create_from_file(path);
    assert(m.cells == NULL);
    assert(remove(path) == 0);
}

static void test_pacman_sample_map(void) {
    Matrix map = matrix_create_from_file("tests/pacman_map.txt");
    assert(map.cells != NULL && map.rows == 31 && map.cols == 28);

    for (int i = 0; i < map.rows * map.cols; i++) {
        const MatrixCell *cell = &map.cells[i];
        assert(cell->active);
        assert(cell->value == 0 || cell->value == 1);
        if (cell->value == 1) {
            assert(cell->r == 40 && cell->g == 80 && cell->b == 255);
        } else {
            assert(cell->r == 0 && cell->g == 0 && cell->b == 0);
        }
    }
    matrix_free(&map);
}

static void test_create_from_array_and_rotation(void) {
    const MatrixCell cells[] = {
        {true, 1, 1, 2, 3}, {false, 2, 4, 5, 6}, {true, 3, 7, 8, 9},
        {false, 4, 10, 11, 12}, {true, 5, 13, 14, 15}, {false, 6, 16, 17, 18}
    };
    Matrix m = matrix_create_from_array(2, 3, cells);
    assert(m.cells != NULL);
    assert(m.cells[4].value == 5 && m.cells[4].active);

    Matrix cw = matrix_rotated(&m, MATRIX_ROTATE_CW);
    assert(cw.rows == 3 && cw.cols == 2);
    assert(cw.cells[0].value == 4);
    assert(cw.cells[1].value == 1);
    assert(cw.cells[2].value == 5);
    assert(cw.cells[3].value == 2);
    assert(cw.cells[4].value == 6);
    assert(cw.cells[5].value == 3);

    Matrix ccw = matrix_rotated(&m, MATRIX_ROTATE_CCW);
    assert(ccw.rows == 3 && ccw.cols == 2);
    assert(ccw.cells[0].value == 3);
    assert(ccw.cells[1].value == 6);
    assert(ccw.cells[2].value == 2);
    assert(ccw.cells[3].value == 5);
    assert(ccw.cells[4].value == 1);
    assert(ccw.cells[5].value == 4);

    matrix_rotate(&m, MATRIX_ROTATE_CW);
    assert(m.rows == 3 && m.cols == 2);
    assert(m.cells[0].value == 4 && m.cells[1].value == 1);

    matrix_free(&m);
    matrix_free(&cw);
    matrix_free(&ccw);
}

static void test_move_row_and_overlap(void) {
    Matrix m = matrix_create_empty(3, 2);
    m.cells[0] = (MatrixCell){true, 10, 1, 2, 3};
    m.cells[1] = (MatrixCell){true, 11, 1, 2, 3};
    matrix_move_row(&m, 0, 2);
    assert(!m.cells[0].active && !m.cells[1].active);
    assert(m.cells[4].active && m.cells[4].value == 10);
    assert(m.cells[5].active && m.cells[5].value == 11);
    matrix_move_row(&m, 1, 1);
    assert(!m.cells[2].active && !m.cells[3].active);

    Matrix piece = matrix_create_empty(2, 2);
    piece.cells[0].active = true;
    assert(!matrix_overlap(&piece, &m, 0, 0, NULL, NULL));
    assert(matrix_overlap(&piece, &m, 2, 0, NULL, NULL));
    assert(matrix_overlap(&piece, &m, -1, 0, NULL, NULL));
    piece.cells[1].active = true;
    assert(matrix_overlap(&piece, &m, 0, 1, NULL, NULL));
    matrix_free(&piece);
    matrix_free(&m);
}

static void test_copy_row_and_null_guards(void) {
    Matrix m = matrix_create_empty(3, 2);
    m.cells[0] = (MatrixCell){true, 10, 1, 2, 3};
    m.cells[1] = (MatrixCell){false, 11, 4, 5, 6};

    matrix_copy_row(&m, 0, 2);
    assert(m.cells[0].active && m.cells[0].value == 10);
    assert(!m.cells[1].active && m.cells[1].value == 11);
    assert(m.cells[4].active && m.cells[4].value == 10);
    assert(!m.cells[5].active && m.cells[5].value == 11);

    matrix_copy_row(&m, 0, 0);
    matrix_copy_row(&m, -1, 1);
    matrix_copy_row(&m, 0, 3);
    assert(m.cells[0].active && m.cells[4].active);

    Matrix invalid = {1, 1, NULL};
    assert(matrix_get_cell(NULL, 0, 0) == NULL);
    assert(matrix_get_cell(&invalid, 0, 0) == NULL);
    assert(matrix_get_row_first_cell(NULL, 0) == NULL);
    assert(matrix_get_row_first_cell(&invalid, 0) == NULL);
    assert(matrix_row_has_inactive_cells(NULL, 0));
    assert(matrix_row_has_inactive_cells(&invalid, 0));
    assert(matrix_overlap(NULL, &m, 0, 0, NULL, NULL));
    matrix_copy_row(NULL, 0, 1);
    matrix_move_row(NULL, 0, 1);
    matrix_merge(NULL, &m, 0, 0);
    matrix_set_cell_value(NULL, 0, 0, 1);
    matrix_set_cell_color(NULL, 0, 0, 1, 2, 3);

    matrix_free(&m);
}

static void test_collapse_rows(void) {
    Matrix m = matrix_create_empty(5, 1);
    for (int row = 0; row < m.rows; row++) {
        m.cells[row].active = true;
        m.cells[row].value = row + 1;
    }

    const bool remove_rows[] = {true, false, true, false, true};
    assert(matrix_collapse_rows(&m, remove_rows) == 3);
    assert(!m.cells[0].active && m.cells[0].value == 0);
    assert(!m.cells[1].active && m.cells[1].value == 0);
    assert(!m.cells[2].active && m.cells[2].value == 0);
    assert(m.cells[3].active && m.cells[3].value == 2);
    assert(m.cells[4].active && m.cells[4].value == 4);
    assert(matrix_collapse_rows(NULL, remove_rows) == 0);
    assert(matrix_collapse_rows(&m, NULL) == 0);

    matrix_free(&m);
}

static void test_overlap_predicate(void) {
    Matrix source = matrix_create_empty(1, 1);
    Matrix destination = matrix_create_empty(1, 1);
    source.cells[0] = (MatrixCell){false, 7, 0, 0, 0};
    destination.cells[0] = (MatrixCell){false, 7, 0, 0, 0};
    int target_value = 7;

    assert(!matrix_overlap(&source, &destination, 0, 0, NULL, NULL));
    assert(matrix_overlap(&source, &destination, 0, 0,
                          cell_has_value, &target_value));
    target_value = 8;
    assert(!matrix_overlap(&source, &destination, 0, 0,
                           cell_has_value, &target_value));
    target_value = 7;
    assert(matrix_overlap(&source, &destination, -1, 0,
                          cell_has_value, &target_value));

    matrix_free(&source);
    matrix_free(&destination);
}

static void test_merge(void) {
    Matrix piece = matrix_create_empty(2, 2);
    Matrix board = matrix_create_empty(3, 3);
    piece.cells[0] = (MatrixCell){true, 7, 100, 110, 120};
    piece.cells[3] = (MatrixCell){true, 9, 130, 140, 150};
    board.cells[4] = (MatrixCell){true, 2, 1, 1, 1};

    matrix_merge(&piece, &board, 1, 1);
    assert(board.cells[4].value == 7);
    assert(board.cells[8].value == 9);
    assert(!board.cells[5].active);
    assert(!board.cells[7].active);

    matrix_free(&piece);
    matrix_free(&board);
}

int main(void) {
    test_create_access_and_setters();
    test_create_from_file();
    test_pacman_sample_map();
    test_create_from_array_and_rotation();
    test_move_row_and_overlap();
    test_copy_row_and_null_guards();
    test_collapse_rows();
    test_overlap_predicate();
    test_merge();
    puts("matrix tests passed");
    return 0;
}
