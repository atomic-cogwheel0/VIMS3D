#include "worldobj.h"

vec3f remove_y_data(vec3f v) {
	return ivec3f(v.x, 0, v.z);
}

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

int add_tank(world_obj *tank, llist l) {
	common_add_object_with_mesh(tank, l);
	tank->mesh->yaw = float2f(90*DEG2RAD_MULT);
	return S_SUCCESS;
}

#pragma inline(acos_f)
fixed acos_f(fixed f) {;
	return float2f(acos(f2float(f)));
}

int tick_tank(world_obj *tank, llist l, world_obj *player, fixed timescale) {
	camera *pl_cam;
	vec3f cam_pos_noy, tank_pos_noy;
	vec3f cam_to_tank, z_axis;
	fixed prod;
	pl_cam = (*(camera **)player->data);

	cam_pos_noy = remove_y_data(pl_cam->pos);
	tank_pos_noy = remove_y_data(subvv(tank->mesh->pos, tank->mesh->ctr));

	cam_to_tank = normalize(subvv(tank_pos_noy, cam_pos_noy));
	z_axis = ivec3f(0, 0, int2f(1));

	tank->mesh->yaw = float2f(90*DEG2RAD_MULT) + acos_f(dotp(cam_to_tank, z_axis));
	
	// move tank towards player (does not rotate yet)
	tank->mesh->pos = addvv(tank->mesh->pos, mulvf(normalize(subvv(cam_pos_noy, tank_pos_noy)), divfi(timescale, 8)));
}