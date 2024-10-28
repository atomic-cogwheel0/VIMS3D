#ifndef WORLDOBJ_H
#define WORLDOBJ_H

#include "world.h"

// create a new static mesh obj with the default handlers (does not do anything on tick)
world_obj iworld_obj_static_mesh(uint8_t t, mesh *m);

// finds angle between the horizontal components of two vectors (useful for orienting an object towards another)
fixed angle_horizontal_plane(vec3f u, vec3f v);

// find closest object to a given type of world object (returns NULL if not found); sets dist_found (nullable) to distance of closest object or FIXED_MAX if not found
world_obj *find_closest_object(world_obj *obj, llist l, int type_to_find, fixed *dist_found);

// move and rotate obj towards dir with speed (0 means no movement, negatives mean away from dir); with check_collision set, no movement if collision occurs
int move_rot_towards(world_obj *obj, world_obj *dir, fixed speed, bool check_collision);

// handle gravity of a given object in the given tick, returns whether obj has hit the ground; sets status (NULLable) to S_SUCCESS normally
bool fall_tick(world_obj *obj, llist l, fixed timescale, int *status);

// ---- specific handlers ----

// a TANK object's handlers
int add_tank(world_obj *tank, llist l);
// move towards nearest PERSON
int tick_tank(world_obj *tank, llist l, world_obj *player, fixed timescale);

// stores pathfinding data for a PERSON world object
typedef struct { 
	vec3f pathfind_target;
	bool should_move;
} person_data_t;

// a PERSON object's handlers
int add_person(world_obj *person, llist l);
int del_person(world_obj *person, llist l);
// flee nearest TANK
int tick_person(world_obj *person, llist l, world_obj *player, fixed timescale);

#endif