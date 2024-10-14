#ifndef _WORLD_H
#define _WORLD_H

#include <stdlib.h>

#include "VIMS_defs.h"
#include "mesh.h"

// world_obj types
#define WORLDOBJ_NULL 0
#define WORLDOBJ_PLAYER 1
#define WORLDOBJ_TANK 2
#define WORLDOBJ_TREE 3

typedef struct _node {
	struct _node *prev;
	struct _node *next;

	struct _wobj *data;
} node;

typedef struct {
	struct _node *head;
	struct _node *tail;
} llist;

typedef struct _wobj {
	uint8_t type;

	uuid_t mesh_id;
	mesh *mesh;

	int (*add_obj)(struct _wobj *obj, llist l);
	int (*del_obj)(struct _wobj *obj, llist l);
	int (*tick_obj)(struct _wobj *obj, llist l, struct _wobj *pl, fixed timescale);
} world_obj;

world_obj iworld_obj(uint8_t t, mesh *m, int (*a)(world_obj *, llist), int (*d)(world_obj *, llist), int (*tk)(world_obj *, llist, world_obj *, fixed));
void dworld_obj(world_obj obj);
node *w_register_to_world(world_obj *obj, int *status);
int w_remove_from_world(node *obj);

world_obj *w_getplayer(void);

int w_init(void);

camera *w_setcam(vec3f pos, fixed pitch, fixed yaw);
void w_tick(fixed timescale);

node *alloc_node(world_obj *data);
void free_node(node *n);

void l_insert_after(llist l, node *n, node *at);
void l_insert_before(llist l, node *n, node *at);
void l_append(llist l, node *n);
void l_prepend(llist l, node *n);

void l_rmnode(llist l, node *n);

#endif