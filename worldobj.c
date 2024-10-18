#include "worldobj.h"

int common_add_object_with_mesh(world_obj *obj, llist l) {
	return m_addmesh(obj->mesh, &obj->instance_in_mesharr);
}

int common_del_object_with_mesh(world_obj *obj, llist l) {
	return m_removemesh(&obj->instance_in_mesharr);
}

world_obj iworld_obj_static_mesh(uint8_t t, mesh *m) {
	return iworld_obj(t, m, NULL, common_add_object_with_mesh, common_del_object_with_mesh, NULL);
}

//--------------------------

int tick_person(world_obj *person, llist l, world_obj *player, fixed timescale) {
	return S_SUCCESS;
}

int add_tank(world_obj *tank, llist l) {
	common_add_object_with_mesh(tank, l);
	tank->mesh->yaw = float2f(90*DEG2RAD_MULT);
	return S_SUCCESS;
}

fixed angle_horizontal_plane(vec3f u, vec3f v) {
	fixed dot = mulff(u.x, v.x) + mulff(u.z, v.z);
	fixed det = mulff(u.x, v.z) - mulff(u.z, v.x);
	fixed angle = float2f((float)atan2(f2float(det), f2float(dot)));
	return angle;
}

int tick_tank(world_obj *tank, llist l, world_obj *player, fixed timescale) {
	vec3f person_to_tank, z_axis;
	node *ptr;
	fixed dist, dist_min = FIXED_MAX;

	z_axis = ivec3f(0, 0, int2f(1));

	ptr = l.head;
	while (ptr->next != NULL) {
		if (ptr->data->type == WORLDOBJ_PERSON) {
			person_to_tank = subvv(tank->mesh->pos, ptr->data->mesh->pos);

			dist = magnitude(person_to_tank);
			if (dist < dist_min) dist_min = dist;
		}
		ptr = ptr->next;
	}
	if (dist_min != FIXED_MAX) {
		tank->mesh->yaw = float2f(90*DEG2RAD_MULT) + angle_horizontal_plane(person_to_tank, z_axis);
		tank->mesh->pos = addvv(tank->mesh->pos, mulvf(normalize(neg(person_to_tank)), divfi(timescale, 8)));
	}
}