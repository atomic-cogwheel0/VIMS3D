#include "world.h"

llist wlist;

camera *global_cam = NULL;

collider player_collider;
world_obj player;
mesh player_mesh;
node *player_node;

static uint32_t last_time = 0;
static bool already_ticked = FALSE;

static bool ticks_frozen = FALSE;
static uint32_t frozen_at = 0;

// debug info
static int w_dbg_mesh_cnt;
static int w_dbg_tri_cnt;
static unsigned int us_elapsed;

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

int w_set_cam_pos(camera cam) {
	if (global_cam == NULL) return S_ENULLPTR;

	(*global_cam) = cam;
	return S_SUCCESS;
}

camera *w_getcam(void) {
	return global_cam;
}

world_obj *w_getplayer(void) {
	return &player;
}

// defined in worldobj.c; handles player movement
extern int tick_player(world_obj *the_player, llist l, world_obj *unused, fixed timescale);

// defined in worldobj.c; rotates billboard toward player
extern int tick_billboard(world_obj *bill, llist l, world_obj *player, fixed timescale);

int w_init(void) {
	int status;
	// create a player obj without a mesh
	player_collider = icoll_aabb(ivec3f(float2f(0.5), int2f(2), float2f(0.5)), ivec3f(float2f(-0.5), int2f(0), float2f(-0.5)));
	player_mesh = inullmesh();
	m_setcoll(&player_mesh, &player_collider, 1);
	player = iworld_obj(WORLDOBJ_PLAYER, &player_mesh, &global_cam, NULL, NULL, tick_player);
	player_node = w_register(&player, &status);
	if (status != S_SUCCESS)
		return status;

	global_cam = &player.mesh->pos;

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
	int ret = S_SUCCESS; // the mainloop will update this if any error has occurred
	node *curr_ptr = wlist.head;
	while (curr_ptr != NULL) {
		// call the deleter to free everything gracefully
		if (curr_ptr->obj->del_obj != NULL) {
			if (curr_ptr->obj->del_obj(curr_ptr->obj, wlist) != S_SUCCESS) {
				ret = S_EUNSPECIFIED; // there can be multiple errors, so this function's error reason might not be determinable
			}
		}
		dworld_obj(*(curr_ptr->obj));
		curr_ptr = curr_ptr->next;
	}
	return ret;
}

void w_tick(void) {
	node *curr_ptr;
	fixed timescale;
	uint32_t calc_time;
	uint32_t curr_time = timer_us();

	if (!ticks_frozen) {
		calc_time = curr_time;
	}
	else {
		calc_time = frozen_at;
		ticks_frozen = FALSE;
	}

	// account for possible overflow
	if (calc_time < last_time) {
		us_elapsed = timer_us_max() - last_time + calc_time;
	}
	else {
		us_elapsed = calc_time - last_time;
	}
	if (!already_ticked) {
		us_elapsed = 0;
		already_ticked = TRUE;
	}

	last_time = curr_time;

	a_tick(us_elapsed);

	timescale = int2f(us_elapsed / 7813) / 25; // convert deltatime to scaler; this makes movement speed unaffected by rendering speed

	curr_ptr = wlist.head;
	while (curr_ptr != NULL) {
		// call the object's tick function
		if (curr_ptr->obj->tick_obj != NULL) {
			curr_ptr->obj->tick_obj(curr_ptr->obj, wlist, &player, timescale);
		}
		if (curr_ptr->obj->mesh != NULL) {
			if (curr_ptr->obj->mesh->is_billboard) {
				tick_billboard(curr_ptr->obj, wlist, &player, timescale);
			}
		}
		curr_ptr = curr_ptr->next;
	}
}

// run this at the end of a tick
void w_freeze_ticking(void) {
	if (!ticks_frozen) {
		ticks_frozen = TRUE;
		frozen_at = timer_us();
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

int w_render_world(camera *cam) {
	mesh *curr;
	node *curr_ptr = wlist.head;

	if (g_getstatus() != SUBSYS_UP) return S_EDOWN;

	// reset debugging global variables
	w_dbg_mesh_cnt = w_dbg_tri_cnt = 0;

	g_clr_depthbuf();
	g_draw_horizon(cam);

	// iterate over the world object list
	curr_ptr = wlist.head;
	while (curr_ptr != NULL) {
		if (curr_ptr->obj->mesh != NULL && curr_ptr->obj->type != WORLDOBJ_PLAYER) {
			curr = curr_ptr->obj->mesh;
			if (!curr->is_renderable)
				continue;
			w_dbg_tri_cnt += g_rasterize_triangles(curr->mesh_arr, curr->tx_arr, curr->tr_cnt, *cam, curr->pos, curr->ctr);
		}
		w_dbg_mesh_cnt++;
		curr_ptr = curr_ptr->next;
	}
	
	return S_SUCCESS;
}

static char buf[64]; // for sprintf()

void w_print_debug(void) {
	snprintf_light(buf, 64, "%1fms (%1ffps) %dt/%dm", int2f(us_elapsed/100)/10, float2f(1e6f/us_elapsed), w_dbg_tri_cnt, w_dbg_mesh_cnt);
	PrintMini(0, 0, (unsigned char *)buf, 0);
	if (global_cam != NULL) {
		snprintf_light(buf, 64, "%1f %1f %1f %1fp %1fy", global_cam->pos.x,
														 global_cam->pos.y,
														 global_cam->pos.z, rad2deg(global_cam->pitch), rad2deg(global_cam->yaw));
	}
	else {
		snprintf_light(buf, 64, "global_cam is NULL!");
	}
	PrintMini(0, 6, (unsigned char *)buf, 0);
}

// results only make sense in the context of a graphics benchmark
#ifdef BENCHMARK_RASTER
void w_print_bench_result(void) {
	// corrupts us_elapsed, so don't call it during normal runtime
	unsigned int curr_time = timer_us();
	if (curr_time < last_time) {
		us_elapsed = timer_us_max() - last_time + curr_time;
	}
	else {
		us_elapsed = curr_time - last_time;
	}
	// print in microseconds
	snprintf_light(buf, 64, "%d\xe6\x4bs %dt/%dm", us_elapsed, w_dbg_tri_cnt, w_dbg_mesh_cnt);
	PrintMini(0, 0, (unsigned char *)buf, 0);
}
#endif

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