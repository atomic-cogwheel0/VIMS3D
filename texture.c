#include "texture.h"

texture_t *i_tx_anim(tx_data_t *tx, short nframes, unsigned int frame_ms, bool loop, bool imm_start) {
    texture_t *r = (texture_t *) malloc(sizeof(texture_t));
    r->texture = tx;
    r->anim.frame = 0;
    r->anim.nframes = nframes;
    r->anim.frame_ms = frame_ms;
    r->anim.is_loop = loop;
    r->anim.is_running = imm_start;
    return r;
}

texture_t *i_tx_static(tx_data_t *tx) {
    texture_t *r = (texture_t *) malloc(sizeof(texture_t));
    r->texture = tx;
    r->anim.nframes = -1;
    return r;
}

void tx_free(texture_t **t) {
    if (t != NULL && *t != NULL) {
        free(*t);
        *t = NULL;
    }
}