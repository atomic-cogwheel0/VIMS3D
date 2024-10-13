#include "game.h"

/*
 GAME.C:
  game setup, adding meshes, handling input
*/

extern texture_t textures[TX_CNT];

vec3f pos;
fixed pitch, yaw;
float gdelta;
float gspeed;

volatile int gamestate = GAMESTATE_PREINIT;

texture_ptr_t tx[72];
texture_ptr_t ptr[1];
texture_ptr_t ptr2[1];
vec3f vertices[26];
trianglef tankmesh[72];
trianglef personmesh[2];
trianglef treemesh[2];
mesh_id_t tid, pid;

#ifndef DEBUG_BUILD
toggle_t overlay = {FALSE, FALSE, FALSE};
#else
toggle_t overlay = {TRUE, FALSE, FALSE};
#endif
toggle_t interlace = {0, 0, 0};

void init(void) {
	int i;
	mesh m, m2;
	int g_ret, m_ret;

	pos = ivec3f(float2f(4.2), float2f(2.0), float2f(12.0));
	pitch = float2f(-1.0*DEG2RAD_MULT), yaw = float2f(197.5*DEG2RAD_MULT);
	gdelta = 32.0f*DEG2RAD_MULT;
	gspeed = 1.0f;

	g_ret = g_init();
	m_ret = m_init();

	if (g_ret != G_SUCCESS || m_ret != G_SUCCESS) {
		locate(1, 1);
		Print((unsigned char *)"init() failed!");
		if (g_ret == G_EALLOC) {
			locate(1, 2);
			Print((unsigned char *)"g_init() alloc fail");
		}
		if (m_ret == G_EALLOC) {
			locate(1, 3);
			Print((unsigned char *)"m_init() alloc fail");
		}
		return;
	}


	for (i = 0; i < 72; i++) {
		tx[i] = &textures[TX_WHITE];
	}
	
	tx[1] = tx[5] = tx[2] = tx[6] = &textures[TX_TANKTRACK];
	tx[31] = tx[32] = tx[33] = tx[34] = tx[35] = tx[36] = tx[37] = &textures[TX_BLACK];
	tx[26] = tx[27] = &textures[TX_TANKFRONT];
	tx[28] = tx[29] = &textures[TX_TANKTOP];

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

	tankmesh[0] = itrianglef(vertices[0], vertices[5], vertices[1], 0, 0, 0);
	tankmesh[1] = itrianglef(vertices[1], vertices[5], vertices[2], 0, 0, 0);
	tankmesh[2] = itrianglef(vertices[4], vertices[2], vertices[5], 0, 0, 0);
	tankmesh[3] = itrianglef(vertices[3], vertices[2], vertices[4], 0, 0, 0);
	
	tankmesh[4] = itrianglef(vertices[6], vertices[7], vertices[11], 0, 0, 0);
	tankmesh[5] = itrianglef(vertices[8], vertices[10], vertices[7], 0, 0, 0);
	tankmesh[6] = itrianglef(vertices[11], vertices[7], vertices[10], 0, 0, 0);
	tankmesh[7] = itrianglef(vertices[9], vertices[10], vertices[8], 0, 0, 0);

	tankmesh[8] = itrianglef(vertices[7], vertices[1], vertices[8], 0, 0, 0);
	tankmesh[9] = itrianglef(vertices[2], vertices[8], vertices[1], 0, 0, 0);	
	tankmesh[10] = itrianglef(vertices[2], vertices[3], vertices[8], 0, 0, 0);
	tankmesh[11] = itrianglef(vertices[9], vertices[8], vertices[3], 0, 0, 0);
	tankmesh[12] = itrianglef(vertices[3], vertices[4], vertices[9], 0, 0, 0);
	tankmesh[13] = itrianglef(vertices[10], vertices[9], vertices[4], 0, 0, 0);
	
	tankmesh[14] = itrianglef(vertices[5], vertices[11], vertices[4], 0, 0, 0);
	tankmesh[15] = itrianglef(vertices[10], vertices[4], vertices[11], 0, 0, 0);
	tankmesh[16] = itrianglef(vertices[6], vertices[11], vertices[0], 0, 0, 0);
	tankmesh[17] = itrianglef(vertices[5], vertices[0], vertices[11], 0, 0, 0);
	tankmesh[18] = itrianglef(vertices[7], vertices[6], vertices[1], 0, 0, 0);
	tankmesh[19] = itrianglef(vertices[0], vertices[1], vertices[6], 0, 0, 0);

	
	tankmesh[20] = itrianglef(vertices[13], vertices[12], vertices[14], 0, 0, 0);
	tankmesh[21] = itrianglef(vertices[15], vertices[14], vertices[12], 0, 0, 0);	
	tankmesh[22] = itrianglef(vertices[14], vertices[15], vertices[18], 0, 0, 0);
	tankmesh[23] = itrianglef(vertices[19], vertices[18], vertices[15], 0, 0, 0);
	tankmesh[24] = itrianglef(vertices[18], vertices[19], vertices[17], 0, 0, 0);
	tankmesh[25] = itrianglef(vertices[16], vertices[17], vertices[19], 0, 0, 0);	
	tankmesh[26] = itrianglef(vertices[17], vertices[16], vertices[13], 0, 0, 0);
	tankmesh[27] = itrianglef(vertices[12], vertices[13], vertices[16], 0, 0, 0);

	tankmesh[28] = itrianglef(vertices[17], vertices[13], vertices[18], 0, 0, 0);
	tankmesh[29] = itrianglef(vertices[14], vertices[18], vertices[13], 0, 0, 0);
	

	tankmesh[30] = itrianglef(vertices[21], vertices[22], vertices[20], 0, 0, 0);

	tankmesh[31] = itrianglef(vertices[21], vertices[20], vertices[24], 0, 0, 0);
	tankmesh[32] = itrianglef(vertices[23], vertices[24], vertices[20], 0, 0, 0);	
	tankmesh[33] = itrianglef(vertices[20], vertices[22], vertices[23], 0, 0, 0);
	tankmesh[34] = itrianglef(vertices[25], vertices[23], vertices[22], 0, 0, 0);
	tankmesh[35] = itrianglef(vertices[22], vertices[21], vertices[25], 0, 0, 0);
	tankmesh[36] = itrianglef(vertices[24], vertices[25], vertices[21], 0, 0, 0);

	personmesh[0] = itrianglef(ivec3f(0, int2f(4), 0), ivec3f(int2f(2), int2f(4), 0), ivec3f(0, 0, 0), 0, 0, 0);
	personmesh[1] = itrianglef(ivec3f(int2f(2), 0, 0), ivec3f(0, 0, 0), ivec3f(int2f(2), int2f(4), 0), 0, 0, 0);

	m = imesh(tankmesh, tx, 37, ivec3f(float2f(1.5),0,float2f(12.0)), ivec3f(0,0,0)); //ivec3f(float2f(-3.5), int2f(2), float2f(-2.5)));
	tid = m_addmesh(m);
	m_rotmesh(tid, int2f(90)*DEG2RAD_MULT);

	ptr[0] = &textures[TX_PERSON];

	m2 = ibill(personmesh, ptr, ivec3f(float2f(6.0),0,float2f(5.0)));
	pid = m_addmesh(m2);
	
	treemesh[0] = itrianglef(ivec3f(0, int2f(12), 0), ivec3f(int2f(6), int2f(12), 0), ivec3f(0, 0, 0), 0, 0, 0);
	treemesh[1] = itrianglef(ivec3f(int2f(6), 0, 0), ivec3f(0, 0, 0), ivec3f(int2f(6), int2f(12), 0), 0, 0, 0);

	ptr2[0] = &textures[TX_TREE];

	for (i = 0; i < 5; i++) {
		m2 = ibill(treemesh, ptr2, ivec3f(int2f(6),0,int2f(i*18)));
	    m_addmesh(m2);
		m2 = ibill(treemesh, ptr2, ivec3f(int2f(-6),0,int2f(i*18)));
		m_addmesh(m2);
	}

#ifndef BENCHMARK_RASTER
	SetTimer(TICK_TIMER, TICK_MS, tick);
	gamestate = GAMESTATE_RUNNING;
#else
	tick();
#endif
}

// deallocate buffers, stop timers, set quit status
void quit(void) {
	gamestate = GAMESTATE_QUIT_INPROG;
	KillTimer(TICK_TIMER);
	KillTimer(DRAW_TIMER);
	gamestate = GAMESTATE_QUIT_DONE;
}

void tick(void) {
	vec3f t;
	int dtime;
	float scale;
	fixed speed, delta;
	
	g_coord(pos, pitch, yaw);
	m_coord(pos, pitch, yaw);

	if (m_getstatus() != G_SUBSYS_UP) return;
	dtime = m_rendermeshes(overlay.is_on, interlace.is_on); //1/128 s ticks
	scale = (float)dtime/128.0f*5.0f;

	delta = float2f(gdelta * scale);
	speed = float2f(gspeed * scale);

	//m_movemesh(tid, ivec3f(0, 0, -speed/2));
	
	Bdisp_PutDisp_DD();

    if (IsKeyDown(KEY_CTRL_UP)) {
		pitch -= delta;
	}
	if (IsKeyDown(KEY_CTRL_DOWN)) {
		pitch += delta;
	}
	if (IsKeyDown(KEY_CTRL_RIGHT)) {
		yaw -= delta;
	}
	if (IsKeyDown(KEY_CTRL_LEFT)) {
		yaw += delta;
	}

	pitch = clamp_f(pitch, float2f(-90*DEG2RAD_MULT), float2f(90*DEG2RAD_MULT));

	yaw = mod_f(yaw, float2f(360*DEG2RAD_MULT));

	t = ivec3f(0, 0, 0);
	if (IsKeyDown(KEY_CHAR_8)) {
		t = rot(ivec3f(0, 0, speed), 0, yaw);
	}
	if (IsKeyDown(KEY_CHAR_2)) {
		t = rot(ivec3f(0, 0, speed), 0, yaw + float2f(180*DEG2RAD_MULT));
	}
	if (IsKeyDown(KEY_CHAR_4)) {
		t = rot(ivec3f(0, 0, speed), 0, yaw + float2f(90*DEG2RAD_MULT));
	}
	if (IsKeyDown(KEY_CHAR_6)) {
		t = rot(ivec3f(0, 0, speed), 0, yaw - float2f(90*DEG2RAD_MULT));
	}
	if (IsKeyDown(KEY_CHAR_9)) {
		t = ivec3f(0, speed, 0);
	}
	if (IsKeyDown(KEY_CHAR_3)) {
		t = ivec3f(0, -speed, 0);
	}
	addpvv(&pos, t);

	toggle_rising(&overlay, IsKeyDown(KEY_CTRL_F3));

	toggle_rising(&interlace, IsKeyDown(KEY_CTRL_F4));

	if (IsKeyDown(KEY_CTRL_EXIT)) {
		quit();
	}
}

int *get_gamestate_ptr(void) {
	return &gamestate;
}

void toggle_rising(toggle_t *t, bool state) {
	t->toggle = state;
	if (t->toggle != t->prev_toggle && t->toggle) {
		t->is_on = !t->is_on;
	}
	t->prev_toggle = t->toggle;
}

void toggle_falling(toggle_t *t, bool state) {
	t->toggle = state;
	if (t->toggle != t->prev_toggle && !t->toggle) {
		t->is_on = !t->is_on;
	}
	t->prev_toggle = t->toggle;
}