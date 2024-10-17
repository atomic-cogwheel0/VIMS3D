#ifndef _WORLD_H
#define _WORLD_H

#include <stdlib.h>

#include "VIMS_defs.h"
#include "mesh.h"

// single node of a linked list of world_objects
typedef struct _node {
	struct _node *prev;
	struct _node *next;

	struct _wobj *data;
} node;

// linked list (has a single instance in world.c, world objects are added to it)
typedef struct {
	struct _node *head;
	struct _node *tail;
} llist;

// world_obj types
#define WORLDOBJ_NULL 0
#define WORLDOBJ_PLAYER 1
#define WORLDOBJ_TANK 2
#define WORLDOBJ_PERSON 3
#define WORLDOBJ_TREE 4

// instance of a world object, handled based on its type and function pointers
typedef struct _wobj {
	uint8_t type;

	mesh *mesh; // may be NULL
	mesh **instance_in_mesharr; // set this in add_obj

	void *data; // pointer to basically any data needed for this object

	// function pointers (obj is a ptr to this object, l is the list of world objects, pl is the player)
	int (*add_obj)(struct _wobj *obj, llist l); // runs automatically, immediately after w_register() adds it to its list of objects
	int (*del_obj)(struct _wobj *obj, llist l); // runs automatically, before w_deregister() removes it from the list
	int (*tick_obj)(struct _wobj *obj, llist l, struct _wobj *pl, fixed timescale); // run this every tick, with timescale based on the time it took to render last frame
} world_obj;

// returns a new world object, m is memcpyed, so feel free to modify it afterwards (BUT DO NOT EDIT THE ARRS)
world_obj iworld_obj(uint8_t t, mesh *m, void *dataptr, int (*a)(world_obj *, llist), int (*d)(world_obj *, llist), int (*tk)(world_obj *, llist, world_obj *, fixed));
// delete a world object (frees malloced memory)
void dworld_obj(world_obj obj);

// adds a new world object instance to the linked list (please don't register an object twice)
// returns the pointer of the new node in the list
// calls obj->add_obj()
// sets *status to EALLOC if node alloc failed, else SUCCESS
// obj has to be a static object or on the heap!
node *w_register(world_obj *obj, int *status);
// remove an object (don't tick it anymore, run obj->data->del_obj()) by a pointer to its node
int w_deregister(node *obj);

// run the given function on every object
int w_run_on_every_obj(int (*func)(world_obj *obj, llist l, world_obj *pl, void *data), void *arg);

// get the player w_obj instance
world_obj *w_getplayer(void);

// initialize list, player, bufs
int w_init(void);

// sets the world's camera object to be newcam
int w_setcam(camera *newcam);
// gets the current camera instance of the world
camera *w_getcam(void);

// tick every object, passes timescale as arg to tick_obj()
void w_tick(fixed timescale);


// allocate a new node on the heap
node *alloc_node(world_obj *data);
// free an allocated node (DANGER: does not unlink from the list!)
void free_node(node *n);

// insert node n after the given node 'at' of the linked list l
void l_insert_after(llist l, node *n, node *at);
// insert node n before the given node 'at' of the linked list l
void l_insert_before(llist l, node *n, node *at);

// insert node n after tail the linked list l
void l_append(llist l, node *n);
// insert node n before head of the linked list l
void l_prepend(llist l, node *n);

// unlink node from list (does not call free_node())
void l_rmnode(llist l, node *n);

#endif