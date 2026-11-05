#ifndef PREVIEW_H
#define PREVIEW_H

#include "tetroid.h"
#include <stdbool.h>

typedef struct {
    Tetromino pieces[3];
} PreviewQueue;

/** Allocate a fresh set of three preview pieces. */
bool preview_queue_init(PreviewQueue *queue);

/** Free all preview-piece matrices and reset the queue. */
void preview_queue_clear(PreviewQueue *queue);

/** Replace all previews with newly generated pieces. */
bool preview_queue_refill(PreviewQueue *queue);

/** Transfer the first preview to active_piece and append a new preview. */
bool preview_queue_advance(PreviewQueue *queue, Tetromino *active_piece);

/** Draw the three preview pieces in the dashboard. */
void preview_queue_draw(const PreviewQueue *queue);

#endif
