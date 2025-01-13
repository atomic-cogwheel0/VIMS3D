#include "texture.h"

texture_t *i_tx_anim(tx_data_t *tx, uint8_t nframes, unsigned int frame_us, bool loop, bool imm_start) {
    texture_t *r = (texture_t *) malloc(sizeof(texture_t));
    r->texture = tx;
    r->anim.frame = 0;
    r->anim.nframes = nframes;
    r->anim.frame_us = frame_us;
    r->anim.us_elapsed = 0;
    r->anim.is_loop = loop;
    r->anim.is_running = imm_start;
    return r;
}

texture_t *i_tx_static(tx_data_t *tx) {
    texture_t *r = (texture_t *) malloc(sizeof(texture_t));
    r->texture = tx;
    r->anim.nframes = 1;
    return r;
}

void tx_free(texture_t **t) {
    if (t != NULL && *t != NULL) {
        free(*t);
        *t = NULL;
    }
}

typedef struct _a_data_node {
    anim_data_t *data;
    struct _a_data_node *next;
} a_data_node;

// head->data is not actually used
static a_data_node head = {NULL, NULL};

static int a_append(anim_data_t *ad) {
    a_data_node *new = (a_data_node *) malloc(sizeof(a_data_node));
    a_data_node *ptr = &head;

    if (new == NULL) return S_EALLOC;

    new->data = ad;
    new->next = NULL;

    while (ptr->next != NULL) {
        ptr = ptr->next;
    }

    ptr->next = new;

    return S_SUCCESS;
}

static void a_remove(anim_data_t *ad) {
    a_data_node *prev_next;
    a_data_node *ptr = &head;
    while (ptr->next != NULL) {
        ptr = ptr->next;
        if (ptr->next->data == ad) {
            prev_next = ptr->next;
            ptr->next = ptr->next->next;
            free(prev_next);
            break;
        }
    }
}

void a_dealloc(void) {
    a_data_node *next;
    a_data_node *ptr = head.next;
    if (ptr == NULL) return; // empty list

    while (ptr->next != NULL) {
        next = ptr->next;
        free(ptr);
        ptr = next;
    }
}

texture_t *a_register_texture(texture_t *tx, int *status) {
    int ret;
    ret = a_append(&(tx->anim));
    if (status != NULL) *status = ret;

    return tx;
}

void a_tick(uint32_t elapse_us) {
    a_data_node *ptr = &head;
    int frame_tmp;
    while (ptr->next != NULL) {
        ptr = ptr->next;
        if (ptr->data != NULL) {
            if (ptr->data->nframes == 1) continue; // not animated

            if (!ptr->data->is_loop || ptr->data->frame <= (ptr->data->nframes - 1)) {
                ptr->data->us_elapsed += elapse_us;
                if (ptr->data->us_elapsed >= ptr->data->frame_us) {
                    frame_tmp = ptr->data->frame + ptr->data->us_elapsed/ptr->data->frame_us;
                    frame_tmp %= ptr->data->nframes;
                    ptr->data->us_elapsed = 0;
                    ptr->data->frame = frame_tmp;
                }
            }
        }
    }
}

int a_px_offset(texture_t *tx) {
    a_data_node *ptr = &head;
   
    if (tx->anim.nframes == 1) return 0;

    return tx->anim.frame * tx->texture->w * tx->texture->h;
}