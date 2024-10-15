#include "worldobj.h"

int common_add_object_with_mesh(world_obj *obj, llist l) {
	obj->mesh_id = m_addmesh(*obj->mesh);
	return S_SUCCESS;
}

int common_del_object_with_mesh(world_obj *obj, llist l) {
	m_removemesh(obj->mesh_id);
	return S_SUCCESS;
}

world_obj iworld_obj_static_mesh(uint8_t t, mesh *m) {
	return iworld_obj(t, m, common_add_object_with_mesh, common_del_object_with_mesh, NULL);
}

//--------------------------

int add_tank(world_obj *tank, llist l) {
	common_add_object_with_mesh(tank, l);
	m_rotmesh(tank->mesh_id, int2f(90)*DEG2RAD_MULT);
	return S_SUCCESS;
}