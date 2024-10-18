#include "world.h"

llist wlist;

camera *global_cam = NULL;

world_obj player;
node *player_node;

world_obj iworld_obj(uint8_t t, mesh *m, void *dataptr, int (*a)(world_obj *, llist), int (*d)(world_obj *, llist), int (*tk)(world_obj *, llist, world_obj *, fixed)) {
	world_obj w;
	w.type = t;
	w.mesh = NULL;
	if (m != NULL) {
		// copy mesh to a new separate location
		w.mesh = malloc(sizeof(mesh));
		memcpy(w.mesh, m, sizeof(mesh));
	}
	w.data = dataptr;
	w.add_obj = a;
	w.del_obj = d;
	w.tick_obj = tk;
	return w;
}

void dworld_obj(world_obj w) {
	if (w.mesh != NULL) free(w.mesh);
}

node *w_register(world_obj *obj, int *status) {
	// allocate new node for the object
	node *to_add = alloc_node(obj);

	if (to_add == NULL)
		if (status != NULL)
			*status = S_EALLOC;

	// add it to the world
	l_append(wlist, to_add);

	// call the object's add function
	if (obj->add_obj != NULL) {
		obj->add_obj(to_add->data, wlist);
	}

	if (status != NULL)
		*status = S_SUCCESS;
	return to_add;
}

int w_deregister(node *n) {
	if (n == NULL)
		return S_ENULLPTR;

	// call the object's delete function
	if (n->data->del_obj != NULL) {
		n->data->del_obj(n->data, wlist);
	}

	// remove every trace of existence
	l_rmnode(wlist, n);
	free_node(n);
	return S_SUCCESS;
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

int _tick_player(world_obj *the_player, llist l, world_obj *unused, fixed timescale);

int w_init(void) {
	// create a player obj without a mesh
	player = iworld_obj(WORLDOBJ_PLAYER, NULL, &global_cam, NULL, NULL, _tick_player);
	player_node = w_register(&player, NULL);
	if (player_node == NULL)
		return S_EALLOC;

	wlist.head = player_node;
	wlist.tail = player_node;

	return S_SUCCESS;
}

int w_free_world(void) {
	// free everything and reset linked list
	node *curr_ptr = wlist.head;
	node *prev_ptr = NULL;
	while (curr_ptr != NULL) {
		prev_ptr = curr_ptr;
		curr_ptr = curr_ptr->next;
		free_node(prev_ptr);
	}
	wlist.head = NULL;
	wlist.tail = NULL;
	return S_SUCCESS;
}

void w_tick(fixed timescale) {
	node *curr_ptr = wlist.head;
	while (curr_ptr != NULL) {
		// call the object's tick function
		if (curr_ptr->data->tick_obj != NULL)
			curr_ptr->data->tick_obj(curr_ptr->data, wlist, &player, timescale);
		curr_ptr = curr_ptr->next;
	}
}

int w_run_on_every_obj(int (*func)(world_obj *obj, llist l, world_obj *pl, void *data), void *arg) {
	// iterate over every registered object and run func() on them; the currently processed object is passed as obj
	node *curr_ptr = wlist.head;
	while (curr_ptr != NULL) {
		func(curr_ptr->data, wlist, &player, arg);
		curr_ptr = curr_ptr->next;
	}
}

int _tick_player(world_obj *the_player, llist l, world_obj *unused, fixed timescale) {
	return S_SUCCESS;
}

static char buf[64]; // for sprintf()

int w_render_world(bool debug_overlay, camera *cam) {
	int m_cnt = 0, t_iter, dx, dy, i;
	unsigned int time_s, time_e2, deltaticks, tr_cnt = 0;
	trianglef curr;
	int16_t **depthbuf; // the pixel depth buffer, reset before meshes are rendered
	node *curr_ptr = wlist.head;

	if (g_getstatus() != SUBSYS_UP) return S_EDOWN;

	depthbuf = g_getdepthbuf();

	// time before rendering
	time_s = RTC_GetTicks();

	// clear pixels and depth values
	Bdisp_AllClr_VRAM();
	for (dx = 0; dx < 128; dx++) {
		for (dy = 0; dy < 64; dy++) {
			depthbuf[dy][dx] = 0x7FFF; // FIXED16_MAX
		}
	}
	g_draw_horizon(cam);
	// iterate over the world object list
	while (curr_ptr != NULL) {
		if (curr_ptr->data->mesh != NULL) {
			if (curr_ptr->data->mesh->flag_is_billboard) {
				curr_ptr->data->mesh->yaw = cam->yaw; // rotate billboard towards camera
			}
			if (!curr_ptr->data->mesh->flag_renderable)
				continue;
			g_clrbuf();
			// add every triangle in mesh
			for (t_iter = 0; t_iter < curr_ptr->data->mesh->tr_cnt; t_iter++) {
				// move triangle to mesh coordinates
				curr = transform_tri_from_zero(curr_ptr->data->mesh->mesh_arr[t_iter], neg(curr_ptr->data->mesh->ctr), 0, 0);
				curr = transform_tri_from_zero(curr, curr_ptr->data->mesh->pos, 0, curr_ptr->data->mesh->yaw);

				curr.tx = curr_ptr->data->mesh->tx_arr[curr_ptr->data->mesh->flag_is_billboard?0:t_iter];
				// flip the second texture of billboards
				if (curr_ptr->data->mesh->flag_is_billboard) {
					curr.flip_texture = t_iter == 0 ? FALSE : TRUE;
				}
				g_addtriangle(curr);
			}
#ifdef BENCHMARK_RASTER
			for (i = 0; i < 40; i++)
#endif
				tr_cnt += g_rasterize_buf(cam);
		}
		m_cnt++;
		curr_ptr = curr_ptr->next;
	}
	// time after rendering
	time_e2 = RTC_GetTicks();
	// calculate correct deltatick value
	deltaticks = time_e2-time_s;
	if (deltaticks < 1) deltaticks = 1; 
	//if (deltaticks > 128) deltaticks -= 128; // the emulator sometimes skips a whole second :)

	// print debug data
#ifndef BENCHMARK_RASTER
	if (debug_overlay)
#endif
	{
		sprintf(buf, "%4.1fms (%2.1ffps) %dt/%dm", (deltaticks)*(1000.0/128.0), 1000.0/((deltaticks)*(1000.0/128.0)), tr_cnt, m_cnt);
		PrintMini(0, 0, (unsigned char *)buf, 0);

		sprintf(buf, "%4.1f %4.1f %4.1f %4.1fp %4.1fy",	f2float(cam->pos.x),
														f2float(cam->pos.y),
														f2float(cam->pos.z), f2float(cam->pitch)*RAD2DEG_MULT, f2float(cam->yaw)*RAD2DEG_MULT);
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
		n->data = data;
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