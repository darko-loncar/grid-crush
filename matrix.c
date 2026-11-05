#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/**
 * Rejects invalid dimensions and allocation-size overflow before zeroed allocation.
 */
Matrix matrix_create_empty(int matrix_rows, int matrix_cols) {
    if (matrix_rows <= 0 || matrix_cols <= 0 || (size_t)matrix_rows > SIZE_MAX / (size_t)matrix_cols) return (Matrix){0};
    size_t cell_count = (size_t)matrix_rows * (size_t)matrix_cols;
    if (cell_count > SIZE_MAX / sizeof(MatrixCell)) return (Matrix){0};
    MatrixCell *cells = calloc(cell_count, sizeof(*cells)); // Init null-state cells
    if(!cells) return (Matrix){0, 0, NULL};
    return (Matrix){matrix_rows, matrix_cols, cells};
}

/**
 * Builds zeroed storage first, then copies the complete row-major cell array.
 */
Matrix matrix_create_from_array(int matrix_rows, int matrix_cols, const MatrixCell *cells) {
    if (!cells || matrix_rows <= 0 || matrix_cols <= 0) return (Matrix){0};
    Matrix matrix = matrix_create_empty(matrix_rows, matrix_cols);
    if (!matrix.cells) return (Matrix){0};
    size_t count = (size_t)matrix_rows * (size_t)matrix_cols;
    memcpy(matrix.cells, cells, count * sizeof(*cells));
    return matrix;
}

/**
 * Parses dimensions and exactly one 0/1 token per cell; malformed input releases partial storage.
 */
Matrix matrix_create_from_file(const char *path) {
    if (!path) return (Matrix){0};

    FILE *file = fopen(path, "r");
    if (!file) return (Matrix){0};

    int rows, cols;
    if (fscanf(file, "%d %d", &rows, &cols) != 2) {
        fclose(file);
        return (Matrix){0};
    }

    Matrix matrix = matrix_create_empty(rows, cols);
    if (!matrix.cells) {
        fclose(file);
        return (Matrix){0};
    }

    for (size_t i = 0; i < (size_t)rows * (size_t)cols; i++) {
        char tile;
        if (fscanf(file, " %c", &tile) != 1 || (tile != '0' && tile != '1')) {
            matrix_free(&matrix);
            fclose(file);
            return (Matrix){0};
        }

        bool wall = tile == '1';
        matrix.cells[i] = (MatrixCell){
            .active = true,
            .value = wall ? 1 : 0,
            .r = wall ? 40 : 0,
            .g = wall ? 80 : 0,
            .b = wall ? 255 : 0
        };
    }

    fclose(file);
    return matrix;
}

/**
 * Replaces the matrix only after the rotated copy has been allocated successfully.
 */
void matrix_rotate(Matrix *m, MatrixRotationDirection direction) {
    if (!m || !m->cells) return;
    Matrix rotated = matrix_rotated(m, direction);

    // Allocation failed; preserve the original matrix.
    if (!rotated.cells) return;

    matrix_free(m);
    *m = rotated;
}

/**
 * Copies a row-sized byte range to the destination without modifying the source.
 */
void matrix_copy_row(Matrix *m, int row, int new_row) {
    if (!m || !m->cells || m->rows <= 0 || m->cols <= 0 ||
        row < 0 || row >= m->rows || new_row < 0 || new_row >= m->rows ||
        row == new_row) return;

    size_t row_size = (size_t)m->cols * sizeof(MatrixCell);
    size_t src_idx = (size_t)row * (size_t)m->cols;
    size_t dest_idx = (size_t)new_row * (size_t)m->cols;
    memcpy(&m->cells[dest_idx], &m->cells[src_idx], row_size);
}

/**
 * Moves one row over another, replacing the destination and clearing
 * the source row. Invalid matrices or row indices are ignored.
 */
void matrix_move_row(Matrix *m, int row, int new_row) {
    if (!m || !m->cells || m->rows <= 0 || m->cols <= 0 || row < 0 || row >= m->rows || new_row < 0 || new_row >= m->rows || row == new_row) return;

    size_t row_size = (size_t)m->cols * sizeof(MatrixCell);
    size_t src_idx = (size_t)row * (size_t)m->cols;
    size_t dest_idx = (size_t)new_row * (size_t)m->cols;

    memcpy(&m->cells[dest_idx], &m->cells[src_idx], row_size); // copy cells to destination row (overwrite)
    memset(&m->cells[src_idx], 0, row_size); // Clear source row.
}

/**
 * Compacts retained rows from bottom to top and zeroes the unused top rows.
 */
int matrix_collapse_rows(Matrix *m, const bool *rows_to_remove) {
    if (!m || !m->cells || !rows_to_remove || m->rows <= 0 || m->cols <= 0) {
        return 0;
    }

    size_t row_size = (size_t)m->cols * sizeof(MatrixCell);
    int destination_row = m->rows - 1;
    int rows_removed = 0;

    for (int source_row = m->rows - 1; source_row >= 0; source_row--) {
        if (rows_to_remove[source_row]) {
            rows_removed++;
            continue;
        }

        if (destination_row != source_row) {
            size_t source_index = (size_t)source_row * (size_t)m->cols;
            size_t destination_index = (size_t)destination_row * (size_t)m->cols;
            memcpy(&m->cells[destination_index], &m->cells[source_index], row_size);
        }
        destination_row--;
    }

    while (destination_row >= 0) {
        size_t row_index = (size_t)destination_row * (size_t)m->cols;
        memset(&m->cells[row_index], 0, row_size);
        destination_row--;
    }

    return rows_removed;
}

/**
 * Scans the row's contiguous matrix cell range and. Invalid input is treated as incomplete.
 */
bool matrix_row_has_inactive_cells(const Matrix *m, int row) {
    if (!m || !m->cells || m->rows <= 0 || m->cols <= 0 ||
        row < 0 || row >= m->rows) return true;
    size_t row_start = (size_t)row * (size_t)m->cols;
    for (int col = 0; col < m->cols; col++) {
        if (!m->cells[row_start + (size_t)col].active) return true;
    }
    return false;
}

/**
 * Allocates a result and maps each source coordinate to its rotated destination.
 */
Matrix matrix_rotated(const Matrix *m, MatrixRotationDirection direction)
{
    if (!m || !m->cells || m->rows <= 0 || m->cols <= 0) return (Matrix){0};

    int new_rows = m->rows;
    int new_cols = m->cols;

    if (direction == MATRIX_ROTATE_CW || direction == MATRIX_ROTATE_CCW) {
        new_rows = m->cols;
        new_cols = m->rows;
    }

    Matrix rotated = matrix_create_empty(new_rows, new_cols);
    if (!rotated.cells) return (Matrix){0};

    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {

            int new_row;
            int new_col;

            if (direction == MATRIX_ROTATE_CW) {
                new_row = j;
                new_col = m->rows - 1 - i;
            }
            else if (direction == MATRIX_ROTATE_CCW) {
                new_row = m->cols - 1 - j;
                new_col = i;
            }
            else {
                // No rotation: copy to the same position.
                new_row = i;
                new_col = j;
            }

            size_t old_index = (size_t)i * (size_t)m->cols + (size_t)j;
            size_t new_index = (size_t)new_row * (size_t)rotated.cols + (size_t)new_col;

            rotated.cells[new_index] = m->cells[old_index];
        }
    }
    return rotated;
}

/**
 * Updates the active flag at the validated row-major cell index.
 */
void matrix_set_cell_active(Matrix *m, int row, int col, bool active) {
    if (!m || !m->cells || row < 0 || row >= m->rows || col < 0 || col >= m->cols) return;
    size_t cell_index = (size_t)row * (size_t)m->cols + (size_t)col;
    m->cells[cell_index].active = active;
}


/**
 * Updates the value field after validating storage and coordinates.
 */
void matrix_set_cell_value(Matrix *m, int row, int col, int value) {
    if (!m || !m->cells || row < 0 || row >= m->rows || col < 0 || col >= m->cols) return;
    size_t cell_index = (size_t)row * (size_t)m->cols + (size_t)col;
    m->cells[cell_index].value = value;
}

/**
 * Updates the three color fields after validating storage and coordinates.
 */
void matrix_set_cell_color(Matrix *m, int row, int col, int r, int g, int b) {
    if (!m || !m->cells || row < 0 || row >= m->rows || col < 0 || col >= m->cols) return;
    MatrixCell *cell = &m->cells[(size_t)row * (size_t)m->cols + (size_t)col];
    cell->r = r;
    cell->g = g;
    cell->b = b;
}

/**
 * Validates coordinates and returns the corresponding row-major cell address.
 */
MatrixCell *matrix_get_cell(Matrix *m, int row, int col) {
    if (!m || !m->cells || m->rows <= 0 || m->cols <= 0 || row < 0 || row >= m->rows || col < 0 || col >= m->cols) return NULL;
    return &m->cells[(size_t)row * (size_t)m->cols + (size_t)col];
}

/**
 * Validates the row and returns its first row-major cell address.
 */
MatrixCell *matrix_get_row_first_cell(Matrix *m, int row) {
    if (!m || !m->cells || m->rows <= 0 || m->cols <= 0 || row < 0 || row >= m->rows) return NULL;
    return &m->cells[(size_t)row * (size_t)m->cols];
}

/**
 * Copies active source cells after computing destination coordinates in a wide integer type.
 * Cells outside the destination are skipped, as are inactive source cells.
 */
void matrix_merge(Matrix *m, Matrix *m_dest, int m_dest_row, int m_dest_col) {
    if (!m || !m_dest || !m->cells || !m_dest->cells ||
        m->rows <= 0 || m->cols <= 0 ||
        m_dest->rows <= 0 || m_dest->cols <= 0) return;

    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            size_t cell_index = (size_t)i * (size_t)m->cols + (size_t)j;
            int64_t dest_row = (int64_t)m_dest_row + i;
            int64_t dest_col = (int64_t)m_dest_col + j;

            if (!m->cells[cell_index].active || dest_row < 0 ||
                dest_row >= m_dest->rows || dest_col < 0 ||
                dest_col >= m_dest->cols) continue;

            size_t dest_cell_idx = (size_t)dest_row * (size_t)m_dest->cols + (size_t)dest_col;
            m_dest->cells[dest_cell_idx] = m->cells[cell_index];
        }
    }
}

/**
 * Applies the supplied cell predicate (or active-flag default) before coordinate checks.
 */
bool matrix_overlap(const Matrix *m, const Matrix *m_dest, int m_dest_row, int m_dest_col, MatrixCellPredicate predicate, void *context) {

    if (!m || !m_dest || !m->cells || !m_dest->cells || m->rows <= 0 || m->cols <= 0 || m_dest->rows <= 0 || m_dest->cols <= 0) return true;

    const int src_cols = m->cols;
    const int src_rows = m->rows;
    const int dest_cols = m_dest->cols;
    const int dest_rows = m_dest->rows;
    const MatrixCell *src = m->cells;
    const MatrixCell *dest = m_dest->cells;

    for (int row = 0; row < src_rows; row++) {
        for (int col = 0; col < src_cols; col++) {
            size_t src_index = (size_t)row * (size_t)src_cols + (size_t)col;
            const MatrixCell *cell = &src[src_index];
            bool source_matches = predicate
                ? predicate(cell, context)
                : cell->active;
            if (!source_matches) continue;

            int64_t dest_row = (int64_t)m_dest_row + row;
            int64_t dest_col = (int64_t)m_dest_col + col;
            if (dest_row < 0 || dest_row >= dest_rows || dest_col < 0 || dest_col >= dest_cols) return true;

            size_t dest_index = (size_t)dest_row * (size_t)dest_cols + (size_t)dest_col;
            const MatrixCell *dest_cell = &dest[dest_index];
            bool destination_matches = predicate
                ? predicate(dest_cell, context)
                : dest_cell->active;
            if (destination_matches) return true;
        }
    }

    return false;
}

/**
 * Iterates over row-major storage and prints each cell's active flag.
 */
void matrix_print(const Matrix *m) {
    if (!m || !m->cells || m->rows <= 0 || m->cols <= 0) return;

    for (int row = 0; row < m->rows; row++) {
        for (int col = 0; col < m->cols; col++) {
            size_t index = (size_t)row * (size_t)m->cols + (size_t)col;
            printf("%d", m->cells[index].active);

            if (col < m->cols - 1) {
                printf(" ");
            }
        }
        printf("\n");
    }
}

/**
 * Releases owned cell storage and resets the structure to its empty state.
 */
void matrix_free(Matrix *m) {
    if (!m) return;
    free(m->cells);
    m->cells = NULL;
    m->rows = 0;
    m->cols = 0;
}
