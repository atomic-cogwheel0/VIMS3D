#include "game.h"

/*
 GAME.C:
  game setup, adding meshes, handling input
*/

extern texture_t textures[TX_CNT];

camera game_cam; // camera (player view position)
float gdelta; // player rotation speed
float gspeed; // player movement speed

volatile int gamestate = GAMESTATE_PREINIT;

#define TANK_MODEL_TRI_CNT 37

// arrays for storing mesh data (vertices, triangles, textures)
vec3f vertices[26];
trianglef tank_mesh[TANK_MODEL_TRI_CNT];
texture_ptr_t tank_txarr[TANK_MODEL_TRI_CNT];

trianglef person_mesh[2];
texture_ptr_t person_txarr[1];

trianglef tree_mesh[2];
texture_ptr_t tree_txarr[1];

// objects (these are global because they are referenced by address)
mesh tank_meshobj;
world_obj tank_worldobj;
mesh person_meshobj;
world_obj person_worldobj;
mesh tree_meshobj;
world_obj tree_worldobjs[10];
node *tree_nodes[10];

// if debugging, set overlay to ON by default
#ifndef DEBUG_BUILD
toggle_t overlay = {FALSE, FALSE, FALSE};
#else
toggle_t overlay = {TRUE, FALSE, FALSE};
#endif

void init(void) {
	int i;
	mesh m, m2;

	// prepare globals
	// set initial camera position
	game_cam.pos = ivec3f(float2f(-3.3), float2f(2.0), float2f(-13.0));
	game_cam.pitch = float2f(10.0*DEG2RAD_MULT); 
	game_cam.yaw = float2f(16.2*DEG2RAD_MULT);

	// 32 degrees/s
	gdelta = 32.0f*DEG2RAD_MULT;
	// 1 unit/s
	gspeed = 1.0f;

	// make sure subsystems are initialized
	assert(g_init() == S_SUCCESS);
	assert(m_init() == S_SUCCESS);
	assert(w_init() == S_SUCCESS);

	w_setcam(&game_cam);

	srand(RTC_GetTicks());

	// initialize every texture of every triangle of the tank mesh
	for (i = 0; i < TANK_MODEL_TRI_CNT; i++) {
		tank_txarr[i] = &textures[TX_WHITE];
	}
	tank_txarr[1] = tank_txarr[5] = tank_txarr[2] = tank_txarr[6] = &textures[TX_TANKTRACK];
	tank_txarr[31] = tank_txarr[32] = tank_txarr[33] = tank_txarr[34] = tank_txarr[35] = tank_txarr[36] = tank_txarr[37] = &textures[TX_BLACK];
	tank_txarr[26] = tank_txarr[27] = &textures[TX_TANKFRONT];
	tank_txarr[28] = tank_txarr[29] = &textures[TX_TANKTOP];

	// preset vertices for easier triangle declaration
	vertices[0] = ivec3f(int2f(1), int2f(1), int2f(0));
	vertices[1] = ivec3f(int2f(0), int2f(2), int2f(0));
	vertices[2] = ivec3f(int2f(-6), int2f(2), int2f(0));
	vertices[3] = ivec3f(int2f(-7), int2f(1), int2f(0));
	vertices[4] = ivec3f(int2f(-6), int2f(0), int2f(0));
	vertices[5] = ivec3f(int2f(0), int2f(0), int2f(0));

	vertices[6] = ivec3f(int2f(1), int2f(1), int2f(5));
	vertices[7] = ivec3f(int2f(0), int2f(2), int2f(5));
	vertices[8] = ivec3f(int2f(-6), int2f(2), int2f(5));
	vertices[9] = ivec3f(int2f(-7), int2f(1), int2f(5));
	vertices[10] = ivec3f(int2f(-6), int2f(0), int2f(5));
	vertices[11] = ivec3f(int2f(0), int2f(0), int2f(5));

	vertices[12] = ivec3f(float2f(-1.5), int2f(2), float2f(1));
	vertices[13] = ivec3f(float2f(-1.5), float2f(3.5), float2f(1));
	vertices[14] = ivec3f(float2f(-4.5), float2f(3.5), float2f(1));
	vertices[15] = ivec3f(float2f(-4.5), int2f(2), float2f(1));

	vertices[16] = ivec3f(float2f(-1.5), int2f(2), float2f(4));
	vertices[17] = ivec3f(float2f(-1.5), float2f(3.5), float2f(4));
	vertices[18] = ivec3f(float2f(-4.5), float2f(3.5), float2f(4));
	vertices[19] = ivec3f(float2f(-4.5), int2f(2), float2f(4));

	vertices[20] = ivec3f(int2f(4), float2f(2.8), float2f(2.25));
	vertices[21] = ivec3f(int2f(4), float2f(3.2), float2f(2.5));
	vertices[22] = ivec3f(int2f(4), float2f(2.8), float2f(2.75));
	vertices[23] = ivec3f(int2f(-3), float2f(2.5), float2f(2.25));
	vertices[24] = ivec3f(int2f(-3), float2f(2.9), float2f(2.5));
	vertices[25] = ivec3f(int2f(-3), float2f(2.5), float2f(2.75));

	// create all triangles of the tank model
	tank_mesh[0] = itrianglef(vertices[0], vertices[5], vertices[1], 0, 0);
	tank_mesh[1] = itrianglef(vertices[1], vertices[5], vertices[2], 0, 0);
	tank_mesh[2] = itrianglef(vertices[4], vertices[2], vertices[5], 0, 0);
	tank_mesh[3] = itrianglef(vertices[3], vertices[2], vertices[4], 0, 0);
	
	tank_mesh[4] = itrianglef(vertices[6], vertices[7], vertices[11], 0, 0);
	tank_mesh[5] = itrianglef(vertices[8], vertices[10], vertices[7], 0, 0);
	tank_mesh[6] = itrianglef(vertices[11], vertices[7], vertices[10], 0, 0);
	tank_mesh[7] = itrianglef(vertices[9], vertices[10], vertices[8], 0, 0);

	tank_mesh[8] = itrianglef(vertices[7], vertices[1], vertices[8], 0, 0);
	tank_mesh[9] = itrianglef(vertices[2], vertices[8], vertices[1], 0, 0);	
	tank_mesh[10] = itrianglef(vertices[2], vertices[3], vertices[8], 0, 0);
	tank_mesh[11] = itrianglef(vertices[9], vertices[8], vertices[3], 0, 0);
	tank_mesh[12] = itrianglef(vertices[3], vertices[4], vertices[9], 0, 0);
	tank_mesh[13] = itrianglef(vertices[10], vertices[9], vertices[4], 0, 0);
	
	tank_mesh[14] = itrianglef(vertices[5], vertices[11], vertices[4], 0, 0);
	tank_mesh[15] = itrianglef(vertices[10], vertices[4], vertices[11], 0, 0);
	tank_mesh[16] = itrianglef(vertices[6], vertices[11], vertices[0], 0, 0);
	tank_mesh[17] = itrianglef(vertices[5], vertices[0], vertices[11], 0, 0);
	tank_mesh[18] = itrianglef(vertices[7], vertices[6], vertices[1], 0, 0);
	tank_mesh[19] = itrianglef(vertices[0], vertices[1], vertices[6], 0, 0);

	
	tank_mesh[20] = itrianglef(vertices[13], vertices[12], vertices[14], 0, 0);
	tank_mesh[21] = itrianglef(vertices[15], vertices[14], vertices[12], 0, 0);	
	tank_mesh[22] = itrianglef(vertices[14], vertices[15], vertices[18], 0, 0);
	tank_mesh[23] = itrianglef(vertices[19], vertices[18], vertices[15], 0, 0);
	tank_mesh[24] = itrianglef(vertices[18], vertices[19], vertices[17], 0, 0);
	tank_mesh[25] = itrianglef(vertices[16], vertices[17], vertices[19], 0, 0);	
	tank_mesh[26] = itrianglef(vertices[17], vertices[16], vertices[13], 0, 0);
	tank_mesh[27] = itrianglef(vertices[12], vertices[13], vertices[16], 0, 0);

	tank_mesh[28] = itrianglef(vertices[17], vertices[13], vertices[18], 0, 0);
	tank_mesh[29] = itrianglef(vertices[14], vertices[18], vertices[13], 0, 0);


	tank_mesh[30] = itrianglef(vertices[21], vertices[22], vertices[20], 0, 0);

	tank_mesh[31] = itrianglef(vertices[21], vertices[20], vertices[24], 0, 0);
	tank_mesh[32] = itrianglef(vertices[23], vertices[24], vertices[20], 0, 0);	
	tank_mesh[33] = itrianglef(vertices[20], vertices[22], vertices[23], 0, 0);
	tank_mesh[34] = itrianglef(vertices[25], vertices[23], vertices[22], 0, 0);
	tank_mesh[35] = itrianglef(vertices[22], vertices[21], vertices[25], 0, 0);
	tank_mesh[36] = itrianglef(vertices[24], vertices[25], vertices[21], 0, 0);

	tank_meshobj = imesh(tank_mesh, tank_txarr, TANK_MODEL_TRI_CNT, ivec3f(float2f(1.5), 0, float2f(12.0)), ivec3f(int2f(-3), 0, float2f(2.5f)));
	tank_worldobj = iworld_obj(WORLDOBJ_TANK, &tank_meshobj, NULL, add_tank, del_tank, tick_tank);

	// a billboard has two symmetric halves
	person_mesh[0] = itrianglef(ivec3f(0, int2f(4), 0), ivec3f(int2f(2), int2f(4), 0), ivec3f(0, 0, 0), 0, 0);
	person_mesh[1] = itrianglef(ivec3f(int2f(2), 0, 0), ivec3f(0, 0, 0), ivec3f(int2f(2), int2f(4), 0), 0, 0);

	person_txarr[0] = &textures[TX_PERSON];

	person_meshobj = ibill(person_mesh, person_txarr, ivec3f(int2f(rand()%40-20), 0, int2f(rand()%40-20)));
	person_worldobj = iworld_obj(WORLDOBJ_PERSON, &person_meshobj, NULL, common_add_object_with_mesh, common_del_object_with_mesh, tick_person);

	// also a billboard
	tree_mesh[0] = itrianglef(ivec3f(0, int2f(12), 0), ivec3f(int2f(6), int2f(12), 0), ivec3f(0, 0, 0), 0, 0);
	tree_mesh[1] = itrianglef(ivec3f(int2f(6), 0, 0), ivec3f(0, 0, 0), ivec3f(int2f(6), int2f(12), 0), 0, 0);

	tree_txarr[0] = &textures[TX_TREE];
	
	for (i = 0; i < 10; i++) {
		// the mesh is memcpyed in iworld_obj, so we can reinit tree_meshobj every time, just don't modify tree_mesh or tree_txarr
		tree_meshobj = ibill(tree_mesh, tree_txarr, ivec3f(int2f(rand()%40-20) * 4, 0, int2f(rand()%40-20) * 4));
		tree_worldobjs[i] = iworld_obj_static_mesh(WORLDOBJ_TREE, &tree_meshobj);
	}

	// register the finished worldobjects into the world
	w_register(&tank_worldobj, NULL);
	w_register(&person_worldobj, NULL);
	for (i = 0; i < 10; i++) {
		tree_nodes[i] = w_register(&tree_worldobjs[i], NULL);
	}

	// do a first tick before rendering to setup everything correctly
	w_tick(0);

	// some call might have modified it (called quit()/halt())
	if (gamestate == GAMESTATE_PREINIT) {
#ifndef BENCHMARK_RASTER
		// schedule tick()
		SetTimer(TICK_TIMER, TICK_MS, tick);
		gamestate = GAMESTATE_RUNNING;
#else
		tick();
#endif
	}
}

// deallocate buffers, stop timers, set quit status
void quit(void) {
	gamestate = GAMESTATE_QUIT_INPROG; // game is quitting, but not ready to return to main menu yet
	// must not tick() any more
	KillTimer(TICK_TIMER);
	// deinit every subsystem
	g_dealloc();
	m_dealloc();
	w_free_world();
	gamestate = GAMESTATE_QUIT_DONE; // finished everything
}

jmp_buf jmpbuf; // for halt handling

void halt(void) {
	gamestate = GAMESTATE_ERR;
	KillTimer(TICK_TIMER);
	// deinit every subsystem
	g_dealloc();
	m_dealloc();
	w_free_world();
	longjmp(jmpbuf, 1); // jump back to main (displays error screen)
}

void halt_msg(char *msg) {
	// print the message then halt
	locate(1,2);
	Print((unsigned char *)msg);
	halt();
}

jmp_buf *get_jmpbuf_ptr(void) {
	return &jmpbuf;
}

void tick(void) {
	vec3f t;
	int dtime;
	float scale;
	fixed speed, delta;
	camera *cam;

	if (m_getstatus() != SUBSYS_UP) return;

	cam = w_getcam();

	dtime = m_rendermeshes(overlay.is_on, cam); // 1/128 s ticks
	scale = (float)dtime/128.0f*5.0f; // convert deltatime to scaler; this makes movement speed unaffected by rendering speed

	// TODO: 1/128 s is not enough resolution; noticeable movement speed changes

	w_tick(float2f(scale)); // tick world objects with scaled movements

	// calculate scaled values
	delta = float2f(gdelta * scale);
	speed = float2f(gspeed * scale);
	
	Bdisp_PutDisp_DD();

	// rotation control
    if (IsKeyDown(KEY_CTRL_UP)) {
		cam->pitch -= delta;
	}
	if (IsKeyDown(KEY_CTRL_DOWN)) {
		cam->pitch += delta;
	}
	if (IsKeyDown(KEY_CTRL_RIGHT)) {
		cam->yaw -= delta;
	}
	if (IsKeyDown(KEY_CTRL_LEFT)) {
		cam->yaw += delta;
	}

	// keep in sane range
	cam->pitch = clamp_f(cam->pitch, float2f(-90*DEG2RAD_MULT), float2f(90*DEG2RAD_MULT));
	cam->yaw = mod_f(cam->yaw, float2f(360*DEG2RAD_MULT));

	// player movement is NOT axis aligned, depends on looking direction
	t = ivec3f(0, 0, 0);
	if (IsKeyDown(KEY_CHAR_8)) {
		t = rot(ivec3f(0, 0, speed), 0, cam->yaw);
	}
	if (IsKeyDown(KEY_CHAR_2)) {
		t = rot(ivec3f(0, 0, speed), 0, cam->yaw + float2f(180*DEG2RAD_MULT));
	}
	if (IsKeyDown(KEY_CHAR_4)) {
		t = rot(ivec3f(0, 0, speed), 0, cam->yaw + float2f(90*DEG2RAD_MULT));
	}
	if (IsKeyDown(KEY_CHAR_6)) {
		t = rot(ivec3f(0, 0, speed), 0, cam->yaw - float2f(90*DEG2RAD_MULT));
	}
	if (IsKeyDown(KEY_CHAR_9)) {
		t = ivec3f(0, speed, 0);
	}
	if (IsKeyDown(KEY_CHAR_3)) {
		t = ivec3f(0, -speed, 0);
	}
	cam->pos = addvv(cam->pos, t);

	toggle_rising(&overlay, IsKeyDown(KEY_CTRL_F3));

	if (IsKeyDown(KEY_CTRL_EXIT)) {
		quit();
	}
}

int *get_gamestate_ptr(void) {
	return &gamestate;
}