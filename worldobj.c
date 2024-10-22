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

//--------------------------

int add_tank(world_obj *tank, llist l) {
	tank->mesh->pos.yaw = float2f(90*DEG2RAD_MULT); // the model is oriented on the wrong axis
	return S_SUCCESS;
}

fixed angle_horizontal_plane(vec3f u, vec3f v) {
	fixed dot = mulff(u.x, v.x) + mulff(u.z, v.z); // dot product
	fixed det = mulff(u.x, v.z) - mulff(u.z, v.x); // determinant
	fixed angle = float2f((float)atan2(f2float(det), f2float(dot)));
	return angle;
}

world_obj *find_closest_object(world_obj *obj, llist l, int type_to_find, fixed *dist_found) {
	vec3f to_obj;
	fixed dist, dist_min = FIXED_MAX;
	node *found = NULL;

	node *ptr = l.head;
	while (ptr != NULL) {
		// find matching type object that is not this one
		if (ptr->data->type == type_to_find && ptr->data != obj) {
			// find vector between centers
			to_obj = subvv(addvv(ptr->data->mesh->pos.pos, ptr->data->mesh->ctr), addvv(obj->mesh->pos.pos, obj->mesh->ctr));

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
		return found->data;
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
	static const min_dist = int2f(1); // the distance that counts as reaching a target point
	static const tank_notice_dist = int2f(15); // start fleeing at this distance
	static const tank_safe_dist = int2f(30); // stop fleeing at this distance
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
			person->mesh->pos.pos = addvv(person->mesh->pos.pos, mulvf(normalize(subvv(pdata->pathfind_target, person->mesh->pos.pos)), timescale/2));
		}
	}
}