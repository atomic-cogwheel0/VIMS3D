#include "game.h"

/*
 GAME.C:
  game setup, adding meshes, handling input
*/

extern tx_data_t textures[TX_CNT];

camera game_cam; // camera (player view position)
float gdelta; // player rotation speed
float gspeed; // player movement speed

volatile int gamestate = GAMESTATE_PREINIT;

#define TANK_MODEL_TRI_CNT 37

// arrays for storing mesh data (vertices, triangles, textures)
vec3f vertices[26];
trianglef tank_mesh[TANK_MODEL_TRI_CNT];
texture_t *tank_txarr[TANK_MODEL_TRI_CNT];

texture_t *tx_tank_sides, *tx_tank_track, *tx_tank_barrel, *tx_tank_front, *tx_tank_top;

trianglef person_mesh[2];
texture_t *person_txarr[2];

trianglef tree_mesh[2];
texture_t *tree_txarr[2];

trianglef arrow_mesh[4];
texture_t *arrow_txarr[4];

// objects (these are global because they are referenced by address)
mesh tank_meshobj;
world_obj tank_worldobj;
collider tank_collider;
node *tank_node;

mesh person_meshobj;
world_obj person_worldobj;
collider person_collider;
node *person_node;

mesh tree_meshobj;
world_obj tree_worldobjs[10];
node *tree_nodes[10];

mesh ground_meshobj;
world_obj ground_worldobj;
collider ground_collider;
node *ground_node;

mesh arrow_meshobj;
world_obj arrow_worldobj;
node *arrow_node;

// if debugging, set overlay to ON by default
#ifndef DEBUG_BUILD
toggle_t overlay = {FALSE, FALSE, FALSE};
#else
toggle_t overlay = {TRUE, FALSE, FALSE};
#endif

menuelement_t menu_pause_element_list[3];
menudef_t menu_pause;

void init(void) {
	int i;
	int status;

	// prepare globals
	// set initial camera position
	game_cam.pos = ivec3f(float2f(-3.3), float2f(2.0), float2f(-13.0));
	game_cam.pitch = float2f(10.0*DEG2RAD_MULT);
	game_cam.yaw = float2f(16.2*DEG2RAD_MULT);

	// make sure subsystems are initialized
	assert(g_init() == S_SUCCESS);
	assert(w_init() == S_SUCCESS);

	w_setcam(&game_cam);

#ifndef BENCHMARK_RASTER
	srand(RTC_GetTicks());
#else
	srand(42); // magic number: show same scene for benchmarks
#endif

	// initialize every texture of every triangle of the tank mesh
	tx_tank_sides = i_tx_static(&textures[TX_WHITE]);
	tx_tank_track = i_tx_static(&textures[TX_TANKTRACK]);
	tx_tank_barrel = i_tx_static(&textures[TX_BLACK]);
	tx_tank_front = i_tx_static(&textures[TX_TANKFRONT]);
	tx_tank_top = i_tx_static(&textures[TX_TANKTOP]);

	for (i = 0; i < TANK_MODEL_TRI_CNT; i++) {
		tank_txarr[i] = tx_tank_sides;
	}
	tank_txarr[1] = tank_txarr[5] = tank_txarr[2] = tank_txarr[6] = tx_tank_track;
	tank_txarr[31] = tank_txarr[32] = tank_txarr[33] = tank_txarr[34] = tank_txarr[35] = tank_txarr[36] = tank_txarr[37] = tx_tank_barrel;
	tank_txarr[26] = tank_txarr[27] = tx_tank_front;
	tank_txarr[28] = tank_txarr[29] = tx_tank_top;

	// preset vertices for easier triangle declaration
	vertices[0] = ivec3i(1, 1, 0);
	vertices[1] = ivec3i(0, 2, 0);
	vertices[2] = ivec3i(-6, 2, 0);
	vertices[3] = ivec3i(-7, 1, 0);
	vertices[4] = ivec3i(-6, 0, 0);
	vertices[5] = ivec3i(0, 0, 0);

	vertices[6] = ivec3i(1, 1, 5);
	vertices[7] = ivec3i(0, 2, 5);
	vertices[8] = ivec3i(-6, 2, 5);
	vertices[9] = ivec3i(-7, 1, 5);
	vertices[10] = ivec3i(-6, 0, 5);
	vertices[11] = ivec3i(0, 0, 5);

	vertices[12] = ivec3float(-1.5f, 2.0f, 1.0f);
	vertices[13] = ivec3float(-1.5f, 3.5f, 1.0f);
	vertices[14] = ivec3float(-4.5f, 3.5f, 1.0f);
	vertices[15] = ivec3float(-4.5f, 2.0f, 1.0f);

	vertices[16] = ivec3float(-1.5f, 2.0f, 4.0f);
	vertices[17] = ivec3float(-1.5f, 3.5f, 4.0f);
	vertices[18] = ivec3float(-4.5f, 3.5f, 4.0f);
	vertices[19] = ivec3float(-4.5f, 2.0f, 4.0f);

	vertices[20] = ivec3float(4.0f, 2.8f, 2.25f);
	vertices[21] = ivec3float(4.0f, 3.2f, 2.5f);
	vertices[22] = ivec3float(4.0f, 2.8f, 2.75f);
	vertices[23] = ivec3float(-3.0f, 2.5f, 2.25f);
	vertices[24] = ivec3float(-3.0f, 2.9f, 2.5f);
	vertices[25] = ivec3float(-3.0f, 2.5f, 2.75f);

	// create all triangles of the tank model
	tank_mesh[0] = itrianglef(vertices[0], vertices[5], vertices[1], FALSE);
	tank_mesh[1] = itrianglef(vertices[1], vertices[5], vertices[2], FALSE);
	tank_mesh[2] = itrianglef(vertices[4], vertices[2], vertices[5], FALSE);
	tank_mesh[3] = itrianglef(vertices[3], vertices[2], vertices[4], FALSE);
	
	tank_mesh[4] = itrianglef(vertices[6], vertices[7], vertices[11], FALSE);
	tank_mesh[5] = itrianglef(vertices[8], vertices[10], vertices[7], FALSE);
	tank_mesh[6] = itrianglef(vertices[11], vertices[7], vertices[10], FALSE);
	tank_mesh[7] = itrianglef(vertices[9], vertices[10], vertices[8], FALSE);

	tank_mesh[8] = itrianglef(vertices[7], vertices[1], vertices[8], FALSE);
	tank_mesh[9] = itrianglef(vertices[2], vertices[8], vertices[1], FALSE);
	tank_mesh[10] = itrianglef(vertices[2], vertices[3], vertices[8], FALSE);
	tank_mesh[11] = itrianglef(vertices[9], vertices[8], vertices[3], FALSE);
	tank_mesh[12] = itrianglef(vertices[3], vertices[4], vertices[9], FALSE);
	tank_mesh[13] = itrianglef(vertices[10], vertices[9], vertices[4], FALSE);
	
	tank_mesh[14] = itrianglef(vertices[5], vertices[11], vertices[4], FALSE);
	tank_mesh[15] = itrianglef(vertices[10], vertices[4], vertices[11], FALSE);
	tank_mesh[16] = itrianglef(vertices[6], vertices[11], vertices[0], FALSE);
	tank_mesh[17] = itrianglef(vertices[5], vertices[0], vertices[11], FALSE);
	tank_mesh[18] = itrianglef(vertices[7], vertices[6], vertices[1], FALSE);
	tank_mesh[19] = itrianglef(vertices[0], vertices[1], vertices[6], FALSE);

	
	tank_mesh[20] = itrianglef(vertices[13], vertices[12], vertices[14], FALSE);
	tank_mesh[21] = itrianglef(vertices[15], vertices[14], vertices[12], FALSE);
	tank_mesh[22] = itrianglef(vertices[14], vertices[15], vertices[18], FALSE);
	tank_mesh[23] = itrianglef(vertices[19], vertices[18], vertices[15], FALSE);
	tank_mesh[24] = itrianglef(vertices[18], vertices[19], vertices[17], FALSE);
	tank_mesh[25] = itrianglef(vertices[16], vertices[17], vertices[19], FALSE);
	tank_mesh[26] = itrianglef(vertices[17], vertices[16], vertices[13], FALSE);
	tank_mesh[27] = itrianglef(vertices[12], vertices[13], vertices[16], FALSE);

	tank_mesh[28] = itrianglef(vertices[17], vertices[13], vertices[18], FALSE);
	tank_mesh[29] = itrianglef(vertices[14], vertices[18], vertices[13], FALSE);


	tank_mesh[30] = itrianglef(vertices[21], vertices[22], vertices[20], FALSE);

	tank_mesh[31] = itrianglef(vertices[21], vertices[20], vertices[24], FALSE);
	tank_mesh[32] = itrianglef(vertices[23], vertices[24], vertices[20], FALSE);
	tank_mesh[33] = itrianglef(vertices[20], vertices[22], vertices[23], FALSE);
	tank_mesh[34] = itrianglef(vertices[25], vertices[23], vertices[22], FALSE);
	tank_mesh[35] = itrianglef(vertices[22], vertices[21], vertices[25], FALSE);
	tank_mesh[36] = itrianglef(vertices[24], vertices[25], vertices[21], FALSE);

	// a spherical collider that has a radius so that it reaches the tip of the tank tracks
	tank_collider = icoll_sphere(ivec3float(-3.0f, 0.0f, 2.5f), magnitude(vertices[3]));

	tank_meshobj = imesh(tank_mesh, tank_txarr, TANK_MODEL_TRI_CNT, ivec3float(1.5f, 0.0f, 12.0f), ivec3float(-3.0f, 0.0f, 2.5f));
	m_setcoll(&tank_meshobj, &tank_collider, 1);
	tank_worldobj = iworld_obj(WORLDOBJ_TANK, &tank_meshobj, NULL, add_tank, NULL, tick_tank);

	// a billboard has two symmetric halves
	person_mesh[0] = itrianglef(ivec3i(-1, 4, 0), ivec3i(1, 4, 0), ivec3i(-1, 0, 0), FALSE);
	person_mesh[1] = itrianglef(ivec3i(1, 0, 0), ivec3i(-1, 0, 0), ivec3i(1, 4, 0), TRUE);

	person_txarr[0] = person_txarr[1] = i_tx_static(&textures[TX_PERSON]);

	// a simple rectangular prism that has an area of 2x2 units
	person_collider = icoll_aabb(ivec3i(-1, 0, -1), ivec3i(1, 4, 1));

	person_meshobj = ibill(person_mesh, person_txarr, ivec3i(RANDINT(20, 60), 0, RANDINT(20, 60)));
	m_setcoll(&person_meshobj, &person_collider, 1);
	person_worldobj = iworld_obj(WORLDOBJ_PERSON, &person_meshobj, NULL, add_person, del_person, tick_person);

	// also a billboard
	tree_mesh[0] = itrianglef(ivec3i(-3, 12, 0), ivec3i(3, 12, 0), ivec3i(-3, 0, 0), FALSE);
	tree_mesh[1] = itrianglef(ivec3i(3, 0, 0), ivec3i(-3, 0, 0), ivec3i(3, 12, 0), TRUE);

	tree_txarr[0] = tree_txarr[1] = i_tx_static(&textures[TX_TREE]);
	
	for (i = 0; i < 10; i++) {
		// the mesh is memcpyed in iworld_obj, so we can reinit tree_meshobj every time, just don't modify tree_mesh or tree_txarr
		tree_meshobj = ibill(tree_mesh, tree_txarr, ivec3i(RANDINT(20, 60) * 4, 0, RANDINT(20, 60) * 4));
		tree_worldobjs[i] = iworld_obj_static_mesh(WORLDOBJ_TREE, &tree_meshobj);
	}
	
	// a pretty large piece of ground if you ask me
	ground_collider = icoll_aabb(ivec3i(-100, -100, -100), ivec3i(100, 0, 100));
	// generic init values for something with no triangles
	ground_meshobj = imesh(NULL, NULL, 0, ivec3i(0, 0, 0), ivec3i(0, 0, 0));
	m_setcoll(&ground_meshobj, &ground_collider, 1);

	ground_worldobj = iworld_obj_static_mesh(WORLDOBJ_GROUND, &ground_meshobj);

	status = m_geom_two_sided_rect(arrow_mesh, ivec3i(-1, 2, 0), ivec3i(1, 0, 0), TRUE);
	assert(status == S_SUCCESS);
	// create animation and register it
	arrow_txarr[0] = a_register_texture(i_tx_anim(&textures[TX_ANIM_ARROW], 2, 400000, TRUE, TRUE), &status);
	arrow_txarr[1] = arrow_txarr[2] = arrow_txarr[3] = arrow_txarr[0];
	assert(status == S_SUCCESS);
	
	arrow_meshobj = imesh(arrow_mesh, arrow_txarr, 4, ivec3float(6.5, 0, 6.5), ivec3i(0, 0, 0));
	arrow_worldobj = iworld_obj(WORLDOBJ_MARKER_ARROW, &arrow_meshobj, NULL, NULL, NULL, tick_tank_marker_arrow);

	// register the finished worldobjects into the world
	tank_node = w_register(&tank_worldobj, &status);
	assert(status == S_SUCCESS);
	person_node = w_register(&person_worldobj, &status);
	assert(status == S_SUCCESS);
	for (i = 0; i < 10; i++) {
		tree_nodes[i] = w_register(&tree_worldobjs[i], &status);
		assert(status == S_SUCCESS);
	}
	ground_node = w_register(&ground_worldobj, &status);
	assert(status == S_SUCCESS);
	arrow_node = w_register(&arrow_worldobj, &status);
	assert(status == S_SUCCESS);

	menu_pause_element_list[0] = ielement_centered(NULL, 14, "Game Paused", MENUELEMENT_LABEL);
	menu_pause_element_list[1] = ielement_centered(onclick_closemenu, 26, "Return", MENUELEMENT_BUTTON);
	menu_pause_element_list[2] = ielement_centered(onclick_quit, 38, "Quit Game", MENUELEMENT_BUTTON);

	menu_pause = imenu(menu_pause_element_list, 3, NULL);

	// some call might have modified it (called quit()/halt())
	if (gamestate != GAMESTATE_PREINIT) return;

	gamestate = GAMESTATE_RUNNING;
}

void free_textures(void) {
	tx_free(&person_txarr[0]);
	tx_free(&tree_txarr[0]);
	tx_free(&tx_tank_front);
	tx_free(&tx_tank_sides);
	tx_free(&tx_tank_barrel);
	tx_free(&tx_tank_track);
	tx_free(&tx_tank_top);
	tx_free(&arrow_txarr[0]);
}

// deallocate buffers, stop timers, set quit status
void quit(void) {
	gamestate = GAMESTATE_QUIT_INPROG; // game is quitting, but not ready to return to main menu yet
	free_textures();
	// deinit every subsystem
	g_dealloc();
	w_dall_world_objs();
	w_free_world();
	a_dealloc();
	gamestate = GAMESTATE_QUIT_DONE; // finished everything
}

jmp_buf jmpbuf; // for halt handling

void halt(void) {
	gamestate = GAMESTATE_ERR;
	longjmp(jmpbuf, 1); // jump back to main (displays error screen)
}

void halt_msg(char *msg) {
	// clear screen, print the message then halt
	Bdisp_AllClr_DDVRAM();
	PrintXY(0, 8, (unsigned char *)msg, 0);
	halt();
}

jmp_buf *get_jmpbuf_ptr(void) {
	return &jmpbuf;
}

void tick(void) {
	int ui_menustatus;
	if (gamestate != GAMESTATE_RUNNING) return;

	ui_menustatus = ui_getmenustatus();

	Keyboard_ClrBuffer(); // may alleviate some crashes
	Bdisp_AllClr_VRAM();

	if (ui_menustatus == MENU_OPEN) {
		w_freeze_ticking();
		ui_rendermenu();
		menu_keyboard_handler();
	}
	else {
		w_tick(); // tick world objects
		w_render_world(w_getcam());

		toggle_rising(&overlay, (bool)IsKeyDown(KEY_CTRL_F3));

		if (overlay.is_on) {
			w_print_debug();
		}

		// use the menu
		if (IsKeyDown(KEY_CTRL_EXIT)) {
			ui_entermenu(&menu_pause);
		}
	}

	Bdisp_PutDisp_DD();
}

volatile int *get_gamestate_ptr(void) {
	return &gamestate;
}