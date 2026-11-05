#include "preview.h"
#include "draw.h"
#include "ui.h"

void preview_queue_clear(PreviewQueue *queue) {
    if (!queue) return;
    for (int i = 0; i < 3; i++) {
        matrix_free(&queue->pieces[i].matrix);
        queue->pieces[i] = (Tetromino){0};
    }
}

bool preview_queue_refill(PreviewQueue *queue) {
    if (!queue) return false;

    Tetromino replacement[3] = {0};
    for (int i = 0; i < 3; i++) {
        replacement[i] = generate_random_piece();
        if (!replacement[i].matrix.cells) {
            for (int j = 0; j < 3; j++) {
                matrix_free(&replacement[j].matrix);
            }
            return false;
        }
    }

    preview_queue_clear(queue);
    for (int i = 0; i < 3; i++) {
        queue->pieces[i] = replacement[i];
    }
    return true;
}

bool preview_queue_init(PreviewQueue *queue) {
    if (!queue) return false;
    *queue = (PreviewQueue){0};
    return preview_queue_refill(queue);
}

bool preview_queue_advance(PreviewQueue *queue, Tetromino *active_piece) {
    if (!queue || !active_piece) return false;

    Tetromino replacement = generate_random_piece();
    if (!replacement.matrix.cells) return false;

    *active_piece = queue->pieces[0];
    queue->pieces[0] = queue->pieces[1];
    queue->pieces[1] = queue->pieces[2];
    queue->pieces[2] = replacement;
    return true;
}

void preview_queue_draw(const PreviewQueue *queue) {
    if (!queue) return;

    if (queue->pieces[0].matrix.cells) {
        drawMatrix(&queue->pieces[0].matrix, (int)UI_PANEL_X, UI_PREVIEW_NEXT_Y);
    }
    if (queue->pieces[1].matrix.cells) {
        drawMatrixSmall(&queue->pieces[1].matrix, (int)UI_PREVIEW_X, UI_PREVIEW_SMALL_2_Y);
    }
    if (queue->pieces[2].matrix.cells) {
        drawMatrixSmall(&queue->pieces[2].matrix, (int)UI_PREVIEW_X, UI_PREVIEW_SMALL_3_Y);
    }
}
