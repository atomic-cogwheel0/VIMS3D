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
	node *to_add = alloc_node(obj);

	if (to_add == NULL)
		if (status != NULL)
			*status = S_EALLOC;

	l_append(wlist, to_add);

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

	if (n->data->del_obj != NULL) {
		n->data->del_obj(n->data, wlist);
	}

	l_rmnode(wlist, n);
	free_node(n);
	n = NULL;
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
	player = iworld_obj(WORLDOBJ_PLAYER, NULL, &global_cam, NULL, NULL, _tick_player);
	player_node = w_register(&player, NULL);
	if (player_node == NULL)
		return S_EALLOC;

	wlist.head = player_node;
	wlist.tail = player_node;

	return S_SUCCESS;
}

void w_tick(fixed timescale) {
	node *curr_ptr = wlist.head;
	while (curr_ptr != NULL) {
		if (curr_ptr->data->tick_obj != NULL)
			curr_ptr->data->tick_obj(curr_ptr->data, wlist, &player, timescale);
		curr_ptr = curr_ptr->next;
	}
}

int _tick_player(world_obj *the_player, llist l, world_obj *unused, fixed timescale) {
	return S_SUCCESS;
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