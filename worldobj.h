#ifndef _WORLDOBJ_H
#define _WORLDOBJ_H

#include "world.h"

// default functions for handling static mesh world objects

// calls m_addmesh()
int common_add_object_with_mesh(world_obj *obj, llist l);
// calls m_removemesh()
int common_del_object_with_mesh(world_obj *obj, llist l);

// create a new static mesh obj with the default handlers (does not do anything on tick)
world_obj iworld_obj_static_mesh(uint8_t t, mesh *m);

// ---- specific handlers ----

int tick_person(world_obj *person, llist l, world_obj *player, fixed timescale);

int add_tank(world_obj *tank, llist l);
#define del_tank common_del_object_with_mesh
int tick_tank(world_obj *tank, llist l, world_obj *player, fixed timescale);

#endif