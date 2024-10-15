#include "worldobj.h"

vec3f remove_y_data(vec3f v) {
	return ivec3f(v.x, 0, v.z);
}

int common_add_object_with_mesh(world_obj *obj, llist l) {
	obj->mesh_id = m_addmesh(obj->mesh);
	return S_SUCCESS;
}

int common_del_object_with_mesh(world_obj *obj, llist l) {
	m_removemesh(obj->mesh_id);
	return S_SUCCESS;
}

world_obj iworld_obj_static_mesh(uint8_t t, mesh *m) {
	return iworld_obj(t, m, NULL, common_add_object_with_mesh, common_del_object_with_mesh, NULL);
}

//--------------------------

int add_tank(world_obj *tank, llist l) {
	common_add_object_with_mesh(tank, l);
	m_rotmesh(tank->mesh_id, float2f(90*DEG2RAD_MULT));
	return S_SUCCESS;
}

int tick_tank(world_obj *tank, llist l, world_obj *player, fixed timescale) {
	camera *pl_cam;
	vec3f cam_pos_noy, tank_pos_noy;
	vec3f cam_to_tank, scr_normal;
	pl_cam = (*(camera **)player->data);

	cam_pos_noy = remove_y_data(pl_cam->pos);
	tank_pos_noy = remove_y_data(subvv(tank->mesh->pos, tank->mesh->ctr));

	// move tank towards player (does not rotate yet)
	m_movemesh(tank->mesh_id, mulvf(normalize(subvv(cam_pos_noy, tank_pos_noy)), divfi(timescale, 8)));
}