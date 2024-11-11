#include "worldobj.h"

// currently unused
/*
int common_add_object_with_mesh(world_obj *obj, llist l) {
	return S_SUCCESS;
}

int common_del_object_with_mesh(world_obj *obj, llist l) {
	return S_SUCCESS;
}
*/

world_obj iworld_obj_static_mesh(uint8_t t, mesh *m) {
	return iworld_obj(t, m, NULL, NULL, NULL, NULL);
}

// --- generic world object code ---

fixed angle_horizontal_plane(vec3f u, vec3f v) {
	fixed dot = mulff(u.x, v.x) + mulff(u.z, v.z); // dot product
	fixed det = mulff(u.x, v.z) - mulff(u.z, v.x); // determinant
	// just do it with floats and atan2, this only runs a few times in a tick
	fixed angle = float2f(atan2f(f2float(det), f2float(dot)));
	return angle;
}

world_obj *find_closest_object(world_obj *obj, llist l, int type_to_find, fixed *dist_found) {
	vec3f to_obj;
	fixed dist, dist_min = FIXED_MAX;
	node *found = NULL;

	node *ptr = l.head;
	while (ptr != NULL) {
		// find matching type object that is not this one
		if (ptr->obj->type == type_to_find && ptr->obj != obj) {
			// find vector between centers
			to_obj = subvv(addvv(ptr->obj->mesh->pos.pos, ptr->obj->mesh->ctr), addvv(obj->mesh->pos.pos, obj->mesh->ctr));

			// get distance, then check if it is nearest object
			dist = magnitude(to_obj);
			if (dist < dist_min) {
				dist_min = dist;
				found = ptr;
			}
		}
		ptr = ptr->next;
	}

	if (found != NULL) {
		if (dist_found != NULL) {
			*dist_found = dist_min;
		}
		return found->obj;
	}
	return NULL;
}

static const vec3f z_axis = {0, 0, int2f(1)}; //ivec3f(0, 0, int2f(1));

int move_rot_towards(world_obj *obj, world_obj *dir, fixed speed, bool check_collision) {
	vec3f a_to_b = ivec3f(0, 0, 0);
	vec3f prev_pos;

	if (obj == NULL || dir == NULL) return S_ENULLPTR;

	// save to restore after the collision
	prev_pos = obj->mesh->pos.pos;

	// find vector between centers (for correct pointing direction)
	a_to_b = subvv(addvv(obj->mesh->pos.pos, obj->mesh->ctr), addvv(dir->mesh->pos.pos, dir->mesh->ctr));

	if (speed != 0) {
		obj->mesh->pos.pos = addvv(obj->mesh->pos.pos, neg(mulvf(normalize(a_to_b), speed)));
	}
	// rotate in the direction of the vector between the two objects (effectively toward the other one)
	obj->mesh->pos.yaw = angle_horizontal_plane(a_to_b, z_axis);

	if (check_collision && m_collide(obj->mesh, dir->mesh)) {
		obj->mesh->pos.pos = prev_pos;
	}

	return S_SUCCESS;
}

// max "sane" iterator value for the collision backtrack loop if something goes wrong
#define ITER_SANE_BACKTRACK 200

// fall downwards until the ground is reached
bool fall_tick(world_obj *obj, llist l, fixed timescale, int *status) {
	static const fixed g_accel = float2f(-0.2f);
	static const fixed backstep = float2f(0.1f); // the resolution of stepping back when the ground is hit
	bool has_collided = FALSE;
	node *ptr;
	int cnt;

	// accelerate and move object downwards
	obj->g_speed += mulff(g_accel, timescale);
	obj->mesh->pos.pos.y += obj->g_speed;

	// check every GROUND object for collision
	ptr = l.head;
	while (ptr != NULL) {
		if (ptr->obj->type == WORLDOBJ_GROUND) {
			// if a collision occurs, backtrack step by step until the two objects don't collide anymore
			if (m_collide(ptr->obj->mesh, obj->mesh)) {
				has_collided = TRUE;
				cnt = 0;
				do {
					obj->mesh->pos.pos.y += backstep;
				} while (m_collide(ptr->obj->mesh, obj->mesh) && ++cnt < ITER_SANE_BACKTRACK);
			}
		}
		ptr = ptr->next;
	}

	// bounce lol
	if (has_collided) {
		obj->g_speed = 0;
	}

	if (status != NULL)
		status = S_SUCCESS;
	return has_collided;
}

// --- object specific functions ---

int add_tank(world_obj *tank, llist l) {
	tank->mesh->pos.yaw = float2f(90*DEG2RAD_MULT); // the model is oriented on the wrong axis
	return S_SUCCESS;
}

int tick_tank(world_obj *tank, llist l, world_obj *player, fixed timescale) {
	world_obj *nearest_person;

	// find the unsuspecting target
	nearest_person = find_closest_object(tank, l, WORLDOBJ_PERSON, NULL);

	if (nearest_person == NULL)
		tank->mesh->pos.yaw = 90*DEG2RAD_MULT;
	else {
		move_rot_towards(tank, nearest_person, divfi(timescale, 6), TRUE);
		tank->mesh->pos.yaw += float2f(90*DEG2RAD_MULT);
	}

	return S_SUCCESS;
}

int add_person(world_obj *person, llist l) {
	// allocate place for data
	person->data = malloc(sizeof(person_data_t));
	if (person->data == NULL)
		return S_EALLOC;
	// initialize pathfinding target
	((person_data_t *)person->data)->pathfind_target = person->mesh->pos.pos;
	((person_data_t *)person->data)->should_move = FALSE;

	return S_SUCCESS;
}

int del_person(world_obj *person, llist l) {
	free(person->data);
	return S_SUCCESS;
}

int tick_person(world_obj *person, llist l, world_obj *player, fixed timescale) {
	person_data_t *pdata = (person_data_t *)person->data; // interpret data as pathfinding
	static const fixed min_dist = int2f(1); // the distance that counts as reaching a target point
	static const fixed tank_notice_dist = int2f(15); // start fleeing at this distance
	static const fixed tank_safe_dist = int2f(30); // stop fleeing at this distance
	fixed target_dist;
	world_obj *nearest_tank;
	fixed tank_dist;

	// rotate towards nearest tank
	nearest_tank = find_closest_object(person, l, WORLDOBJ_TANK, &tank_dist);
	if (nearest_tank != NULL) {
		move_rot_towards(person, nearest_tank, 0, FALSE);

		// find distance to pathfinding target
		target_dist = magnitude(subvv(pdata->pathfind_target, person->mesh->pos.pos));
		if (target_dist < min_dist) {
			// stop here if tank is outside unsafe area; if tank comes near, find new target
			pdata->should_move = (tank_dist < tank_safe_dist);
			if (pdata->should_move) {
				// find next target by looking away from the tank then rotating a bit (at most 100 degrees in a direction) and finding a point in that direction (at least 5, at most 20 units away)
				pdata->pathfind_target = addvv(pdata->pathfind_target, mulvi(py2vec3f(0, person->mesh->pos.yaw + float2f((rand()%200-100)*DEG2RAD_MULT)), rand()%15+5));
			}
		}
		// if tank comes within notice distance, start moving until it is out of the unsafe distance
		if (tank_dist < tank_notice_dist) {
			pdata->should_move = TRUE;
		}

		// move towards pathfinding target
		if (pdata->should_move) {
			person->mesh->pos.pos = addvv(person->mesh->pos.pos, mulvf(normalize(horiz(subvv(pdata->pathfind_target, person->mesh->pos.pos))), timescale/2));
		}
	}
	return S_SUCCESS;
}

int tick_player(world_obj *the_player, llist l, world_obj *unused, fixed timescale) {
	camera *cam = *(camera **)the_player->data;
	vec3f t;
	static const fixed movement_speed = float2f(1.0f); // the amount the player should move in 1 unit of timescale
	static const fixed rotation_speed = float2f(32.0f*DEG2RAD_MULT); // number of degrees the player should turn in 1 unit of timescale

	fixed speed = mulff(movement_speed, timescale);
	fixed rotation = mulff(rotation_speed, timescale);

	// rotation control
	if (IsKeyDown(KEY_CTRL_UP)) {
		cam->pitch -= rotation;
	}
	if (IsKeyDown(KEY_CTRL_DOWN)) {
		cam->pitch += rotation;
	}
	if (IsKeyDown(KEY_CTRL_RIGHT)) {
		cam->yaw -= rotation;
	}
	if (IsKeyDown(KEY_CTRL_LEFT)) {
		cam->yaw += rotation;
	}

	// keep in sane range
	cam->pitch = clamp_f(cam->pitch, float2f(-90*DEG2RAD_MULT), float2f(90*DEG2RAD_MULT));
	cam->yaw = mod_f(cam->yaw, float2f(360*DEG2RAD_MULT));

	// player movement is NOT axis aligned, depends on looking direction
	t = ivec3i(0, 0, 0);
	if (IsKeyDown(KEY_CHAR_8)) {
		t = addvv(t, rot(ivec3i(0, 0, 1), 0, cam->yaw));
	}
	if (IsKeyDown(KEY_CHAR_2)) {
		t = addvv(t, rot(ivec3i(0, 0, 1), 0, cam->yaw + float2f(180*DEG2RAD_MULT)));
	}
	if (IsKeyDown(KEY_CHAR_4)) {
		t = addvv(t, rot(ivec3i(0, 0, 1), 0, cam->yaw + float2f(90*DEG2RAD_MULT)));
	}
	if (IsKeyDown(KEY_CHAR_6)) {
		t = addvv(t, rot(ivec3i(0, 0, 1), 0, cam->yaw - float2f(90*DEG2RAD_MULT)));
	}
	t = normalize(t); // normalize movement speed in the horizontal axis
	if (IsKeyDown(KEY_CHAR_9)) {
		t = addvv(t, ivec3i(0, 1, 0));
	}
	if (IsKeyDown(KEY_CHAR_3)) {
		t = addvv(t, ivec3i(0, -1, 0));
	}
	t = mulvf(t, speed);
	cam->pos = addvv(cam->pos, t);

	return S_SUCCESS;
}