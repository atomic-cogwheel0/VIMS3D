#include "world.h"

llist wlist;

camera *global_cam = NULL;

world_obj player;
node *player_node;

world_obj iworld_obj(uint8_t type, mesh *m, void *dataptr, int (*add)(world_obj *, llist), int (*del)(world_obj *, llist), int (*tck)(world_obj *, llist, world_obj *, fixed)) {
	world_obj w;
	w.type = type;
	w.mesh = NULL;
	if (m != NULL) {
		// copy mesh to a new separate location
		w.mesh = malloc(sizeof(mesh));
		memcpy(w.mesh, m, sizeof(mesh));
	}
	w.g_speed = int2f(0);
	w.data = dataptr;
	w.add_obj = add;
	w.del_obj = del;
	w.tick_obj = tck;
	return w;
}

void dworld_obj(world_obj w) {
	if (w.mesh != NULL) free(w.mesh);
}

node *w_register(world_obj *obj, int *status) {
	int ret;
	// allocate new node for the object
	node *to_add = alloc_node(obj);

	if (to_add == NULL) {
		ret = S_EALLOC;
	}
	else {
		// add it to the world
		l_append(wlist, to_add);

		// call the object's add function
		if (obj->add_obj != NULL) {
			ret = obj->add_obj(to_add->obj, wlist);
		}
		else {
			ret = S_SUCCESS; // nothing bad happened
		}
	}
	if (status != NULL) *status = ret;
	return to_add;
}

int w_deregister(node *n) {
	int del_ret = S_SUCCESS; // return S_SUCCESS when del_obj == NULL, otherwise the return value of del_obj()

	if (n == NULL)
		return S_ENULLPTR;

	// call the object's delete function
	if (n->obj->del_obj != NULL) {
		del_ret = n->obj->del_obj(n->obj, wlist);
	}

	// remove every trace of existence
	l_rmnode(wlist, n);
	free_node(n);
	return del_ret;
}

int w_setcam(camera *newcam) {
	if (newcam == NULL) return S_ENULLPTR;

	global_cam = newcam;
	return S_SUCCESS;
}

camera *w_getcam(void) {
	return global_cam;
}

world_obj *w_getplayer(void) {
	return &player;
}

int tick_player(world_obj *the_player, llist l, world_obj *unused, fixed timescale);

int w_init(void) {
	int status;
	// create a player obj without a mesh
	player = iworld_obj(WORLDOBJ_PLAYER, NULL, &global_cam, NULL, NULL, tick_player);
	player_node = w_register(&player, &status);
	if (status != S_SUCCESS)
		return status;

	wlist.head = player_node;
	wlist.tail = player_node;

	return S_SUCCESS;
}

int w_free_world(void) {
	// free everything and reset linked list
	node *curr_ptr = wlist.head;
	node *prev_ptr = NULL;
	while (curr_ptr != NULL) {
		// save the pointer before incrementing, as curr_ptr->next won't be available after free_node
		prev_ptr = curr_ptr;
		curr_ptr = curr_ptr->next;
		// free them
		free_node(prev_ptr);
	}
	wlist.head = NULL;
	wlist.tail = NULL;
	return S_SUCCESS;
}

int w_dall_world_objs(void) {
	int del_ret = S_SUCCESS; // return S_SUCCESS when del_obj == NULL, otherwise the return value of del_obj()
	node *curr_ptr = wlist.head;
	while (curr_ptr != NULL) {
		// call the deleter to free everything gracefully
		if (curr_ptr->obj->del_obj != NULL) {
			del_ret = curr_ptr->obj->del_obj(curr_ptr->obj, wlist);
		}
		dworld_obj(*(curr_ptr->obj));
		curr_ptr = curr_ptr->next;
	}
	return del_ret;
}

void w_tick(fixed timescale) {
	node *curr_ptr = wlist.head;
	while (curr_ptr != NULL) {
		// call the object's tick function
		if (curr_ptr->obj->tick_obj != NULL)
			curr_ptr->obj->tick_obj(curr_ptr->obj, wlist, &player, timescale);
		curr_ptr = curr_ptr->next;
	}
}

int w_run_on_every_obj(int (*func)(world_obj *obj, llist l, world_obj *pl, void *data), void *arg) {
	// iterate over every registered object and run func() on them; the currently processed object is passed as obj
	node *curr_ptr = wlist.head;
	while (curr_ptr != NULL) {
		func(curr_ptr->obj, wlist, &player, arg);
		curr_ptr = curr_ptr->next;
	}
	return S_SUCCESS;
}

int tick_player(world_obj *the_player, llist l, world_obj *unused, fixed timescale) {
	return S_SUCCESS;
}

static char buf[64]; // for sprintf()

int w_render_world(bool debug_overlay, camera *cam) {
	int m_cnt = 0, t_iter, dx, dy, i;
	unsigned int time_s, time_e, deltaticks, tr_cnt = 0;
	mesh *curr;
	int16_t **depthbuf; // the pixel depth buffer, reset before meshes are rendered
	node *curr_ptr = wlist.head;

	if (g_getstatus() != SUBSYS_UP) return S_EDOWN;

	depthbuf = g_getdepthbuf();

	// time before rendering
	time_s = RTC_GetTicks();

	curr_ptr = wlist.head;
	// clear depth values
	for (dx = 0; dx < 128; dx++) {
		for (dy = 0; dy < 64; dy++) {
			depthbuf[dy][dx] = 0x7FFF; // FIXED16_MAX
		}
	}
	g_draw_horizon(cam);
	// iterate over the world object list
	while (curr_ptr != NULL) {
		if (curr_ptr->obj->mesh != NULL) {
			curr = curr_ptr->obj->mesh;
			if (curr->flag_is_billboard) {
				curr->pos.yaw = cam->yaw; // rotate billboard towards camera
			}
			if (!curr->flag_renderable)
				continue;
			tr_cnt += g_rasterize_triangles(curr->mesh_arr, curr->tx_arr, curr->tr_cnt, *cam, curr->pos, curr->ctr);
		}
		m_cnt++;
		curr_ptr = curr_ptr->next;
	}

	// time after rendering
	time_e = RTC_GetTicks();
	// calculate correct deltatick value
	deltaticks = time_e-time_s;
	if (deltaticks < 1) deltaticks = 1; // delta is used as a divisor later
	//if (deltaticks > 128) deltaticks -= 128; // the emulator sometimes skips a whole second :)

	// print debug data
#ifndef BENCHMARK_RASTER
	if (debug_overlay)
#endif
	{
		snprintf_light(buf, 64, "%1fms (%1ffps) %dt/%dm", int2f(deltaticks)*1000/128, int2f(128)/deltaticks, tr_cnt, m_cnt);
		PrintMini(0, 0, (unsigned char *)buf, 0);

		snprintf_light(buf, 64, "%1f %1f %1f %1fp %1fy", cam->pos.x,
		                                                 cam->pos.y,
		                                                 cam->pos.z, rad2deg(cam->pitch), rad2deg(cam->yaw));
		PrintMini(0, 6, (unsigned char *)buf, 0);
	}

	return deltaticks;
}

//------------ linked list code ------------

node *alloc_node(world_obj *data) {
	node *n = (node *) malloc(sizeof(node));

	if (n != NULL) {
		n->next = NULL;
		n->prev = NULL;
		n->obj = data;
	}
	return n;
}

void free_node(node *n) {
	if (n == NULL)
		return;
	free(n);
}

void l_insert_after(llist l, node *n, node *at) {
	if (n == NULL) return;
	if (at == NULL) return;

	n->prev = at;
	n->next = at->next;
	if (at->next == NULL) {
		l.tail = n;
	}
	else {
		at->next->prev = n;
	}
	at->next = n;
}

void l_insert_before(llist l, node *n, node *at) {
	if (n == NULL) return;
	if (at == NULL) return;

	n->next = at;
	n->prev = at->prev;
	if (at->prev == NULL) {
		l.head = n;
	}
	else {
		at->prev->next = n;
	}
	at->prev = n;
}

void l_append(llist l, node *n) {
	l_insert_after(l, n, l.tail);
}

void l_prepend(llist l, node *n) {
	l_insert_before(l, n, l.head);
}

void l_rmnode(llist l, node *n) {
	if (n == NULL) return;

	if (n->prev == NULL) {
		l.head = n->next;
	}
	else {
		n->prev->next = n->next;
	}
	if (n->next == NULL) {
		l.tail = n->prev;
	}
	else {
		n->next->prev = n->prev;
	}
}