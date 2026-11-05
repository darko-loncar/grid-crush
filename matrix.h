/**
 * @file matrix.h
 * @brief Public API for creating, inspecting, transforming, and freeing cell matrices.
 *
 * A Matrix owns a row-major array of MatrixCell values. The library provides
 * constructors, cell access and mutation, rotation, row operations, overlap
 * tests with customizable predicates, merging, printing, and cleanup.
 *
 * @author Darko Lončar
 * @copyright 2026
 *
 * @par Function overview
 * - Creation:      matrix_create_empty(), matrix_create_from_array(), matrix_create_from_file(), matrix_rotated()
 * - Access:        matrix_get_cell(), matrix_get_row_first_cell()
 * - Mutation:      matrix_rotate(), matrix_set_cell_active(), matrix_set_cell_value()
 *                  matrix_set_cell_color(), matrix_merge(), matrix_copy_row(), matrix_move_row()
 *                  matrix_collapse_rows()
 * - Inspection:    matrix_overlap(), matrix_row_has_inactive_cells()
 * - Debug:         matrix_print()
 * - Cleanup:       matrix_free()
 */
#ifndef MATRIX_H
#define MATRIX_H

#include <stdbool.h>

/** Direction used when rotating a matrix. */
typedef enum {
    MATRIX_ROTATE_CW  = 1,
    MATRIX_ROTATE_CCW = -1
} MatrixRotationDirection;

/** A matrix cell's occupancy, value, and RGB color. */
typedef struct {
    bool active;
    int value;
    int r;
    int g;
    int b;
} MatrixCell;

/** Predicate that decides whether a cell participates in overlap checks. */
typedef bool (*MatrixCellPredicate)(const MatrixCell *cell, void *context);

/** Row-major rectangular array of cells. */
typedef struct {
    int rows;
    int cols;
    MatrixCell *cells;
} Matrix;

/**
 * Allocates a zero-initialized matrix.
 * @param matrix_rows Number of rows; must be positive.
 * @param matrix_cols Number of columns; must be positive.
 * @return The allocated matrix, or a zeroed invalid matrix on invalid dimensions or allocation failure.
 * @code
 * Matrix m = matrix_create_empty(2, 3);
 * @endcode
 */
Matrix matrix_create_empty(int matrix_rows, int matrix_cols);

/**
 * Allocates a matrix and copies its cells from a row-major array.
 * @param matrix_rows Number of rows; must be positive.
 * @param matrix_cols Number of columns; must be positive.
 * @param cells Source array containing at least matrix_rows * matrix_cols cells.
 * @return The copied matrix, or a zeroed matrix on invalid input or allocation failure.
 * @code
 * MatrixCell cells[2] = {0};
 * Matrix m = matrix_create_from_array(1, 2, cells);
 * @endcode
 */
Matrix matrix_create_from_array(int matrix_rows, int matrix_cols, const MatrixCell *cells);

/**
 * Loads dimensions followed by at least one 0/1 tile per cell; whitespace is ignored and trailing input is ignored.
 * Loaded cells are active; value 1 is colored blue and value 0 black.
 * @param path Path to the input file.
 * @return The loaded matrix, or a zeroed invalid matrix if the path, file, dimensions, or tile data are invalid.
 * @code
 * Matrix level = matrix_create_from_file("levels/level1.txt");
 * @endcode
 */
Matrix matrix_create_from_file(const char *path);

/**
 * Returns a newly allocated rotated copy; the input matrix is unchanged. Unknown direction values produce an unchanged copy.
 * @param m Matrix to rotate.
 * @param direction Clockwise or counter-clockwise rotation.
 * @return The rotated matrix, or a zeroed invalid matrix on invalid matrix input or allocation failure.
 * @code
 * Matrix rotated = matrix_rotated(&m, MATRIX_ROTATE_CW);
 * @endcode
 */
Matrix matrix_rotated(const Matrix *m, MatrixRotationDirection direction);

/**
 * Returns a mutable pointer to a cell at the given coordinates.
 * @param m Matrix to access.
 * @param row Zero-based row index.
 * @param col Zero-based column index.
 * @return Cell pointer, or NULL if the matrix or coordinates are invalid.
 * @code
 * MatrixCell *cell = matrix_get_cell(&m, 0, 1);
 * @endcode
 */
MatrixCell *matrix_get_cell(Matrix *m, int row, int col);

/**
 * Returns a mutable pointer to the first cell in a row.
 * @param m Matrix to access.
 * @param row Zero-based row index.
 * @return Pointer to the row's first cell, or NULL if the matrix or row is invalid.
 * @code
 * MatrixCell *first = matrix_get_row_first_cell(&m, 1);
 * @endcode
 */
MatrixCell *matrix_get_row_first_cell(Matrix *m, int row);

/**
 * Tests whether cells selected by the predicate overlap or lie outside the destination.
 * A NULL predicate uses each cell's active flag.
 * @param m Source matrix.
 * @param m_dest Destination matrix.
 * @param m_dest_row Top row of the source placement in the destination.
 * @param m_dest_col Left column of the source placement in the destination.
 * @param predicate Returns true when a cell participates in overlap checks; NULL selects active cells.
 * @param context Opaque value passed to predicate; ignored when predicate is NULL.
 * @return true for overlap, out-of-bounds selected source cells, or invalid matrices; otherwise false.
 * @code
 * bool is_wall(const MatrixCell *cell, void *context) {
 *     return cell->value == *(const int *)context;
 * }
 * int wall_value = 1;
 * bool blocked = matrix_overlap(&piece, &board, row, col, is_wall, &wall_value);
 * @endcode
 */
bool matrix_overlap(const Matrix *m, const Matrix *m_dest, int m_dest_row, int m_dest_col, MatrixCellPredicate predicate, void *context);

/**
 * Tests whether a row contains at least one inactive cell.
 * @param m Matrix to inspect.
 * @param row Zero-based row index.
 * @return true if the row contains an inactive cell or the input is invalid; otherwise false.
 * @code
 * bool incomplete = matrix_row_has_inactive_cells(&m, 1);
 * @endcode
 */
bool matrix_row_has_inactive_cells(const Matrix *m, int row);

/**
 * Sets a cell's active flag.
 * @param m Matrix to modify.
 * @param row Zero-based row index.
 * @param col Zero-based column index.
 * @param active New active state.
 * Invalid matrices or coordinates are ignored.
 * @code
 * matrix_set_cell_active(&m, 0, 1, true);
 * @endcode
 */
void matrix_set_cell_active(Matrix *m, int row, int col, bool active);

/**
 * Sets a cell's value.
 * @param m Matrix to modify.
 * @param row Zero-based row index.
 * @param col Zero-based column index.
 * @param value New cell value.
 * Invalid matrices or coordinates are ignored.
 * @code
 * matrix_set_cell_value(&m, 0, 1, 42);
 * @endcode
 */
void matrix_set_cell_value(Matrix *m, int row, int col, int value);

/**
 * Sets a cell's RGB color components.
 * @param m Matrix to modify.
 * @param row Zero-based row index.
 * @param col Zero-based column index.
 * @param r Red component.
 * @param g Green component.
 * @param b Blue component.
 * Invalid matrices or coordinates are ignored.
 * @code
 * matrix_set_cell_color(&m, 0, 1, 255, 128, 0);
 * @endcode
 */
void matrix_set_cell_color(Matrix *m, int row, int col, int r, int g, int b);

/**
 * Replaces a matrix with its rotated form.
 * @param m Matrix to rotate in place.
 * @param direction Clockwise or counter-clockwise rotation.
 * If the matrix or direction is invalid, or allocation fails, the original is preserved.
 * @code
 * matrix_rotate(&m, MATRIX_ROTATE_CCW);
 * @endcode
 */
void matrix_rotate(Matrix *m, MatrixRotationDirection direction);

/**
 * Copies one row over another, preserving the source and replacing the destination.
 * @param m Matrix to modify.
 * @param row Zero-based source row.
 * @param new_row Zero-based destination row.
 * Invalid matrices, identical rows, or out-of-range rows are ignored.
 * @code
 * matrix_copy_row(&m, 0, 2);
 * @endcode
 */
void matrix_copy_row(Matrix *m, int row, int new_row);

/**
 * Copies one row over another and clears the source row.
 * Destination row is overwritten.
 * @param m Matrix to modify.
 * @param row Zero-based source row.
 * @param new_row Zero-based destination row.
 * Invalid matrices, identical rows, or out-of-range rows are ignored.
 * @code
 * matrix_move_row(&m, 0, 2);
 * @endcode
 */
void matrix_move_row(Matrix *m, int row, int new_row);

/**
 * Removes rows selected by a Boolean mask and compacts remaining rows downward.
 * Vacated rows at the top are reset to zero.
 * @param m Matrix to modify.
 * @param rows_to_remove Array with one entry per matrix row; true marks a row for removal.
 * @return Number of rows removed, or 0 if the matrix or mask is invalid.
 * @code
 * bool remove_rows[3] = {false, true, false};
 * int removed = matrix_collapse_rows(&m, remove_rows);
 * @endcode
 */
int matrix_collapse_rows(Matrix *m, const bool *rows_to_remove);

/**
 * Copies active source cells into the destination at the given offset; inactive source cells are ignored.
 * Active cells placed outside destination bounds are ignored.
 * @param m Source matrix.
 * @param m_dest Destination matrix to modify.
 * @param m_dest_row Top row of the source placement in the destination.
 * @param m_dest_col Left column of the source placement in the destination.
 * Invalid matrices are ignored.
 * @code
 * matrix_merge(&piece, &board, 3, 4);
 * @endcode
 */
void matrix_merge(Matrix *m, Matrix *m_dest, int m_dest_row, int m_dest_col);

/**
 * Prints cell occupancy as space-separated 0/1 values, one row per line.
 * @param m Matrix to print; invalid matrices produce no output.
 * @code
 * matrix_print(&m);
 * @endcode
 */
void matrix_print(const Matrix *m);

/**
 * Frees the cell storage and resets the matrix dimensions and pointer.
 * @param m Matrix to release; NULL is ignored.
 * @code
 * matrix_free(&m);
 * @endcode
 */
void matrix_free(Matrix *m);

#endif // MATRIX_H
