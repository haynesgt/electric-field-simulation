#include "global.h"
#include "app.h"
#include "misc_functions.h"
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <GL/glu.h>
#include <math.h>

#define NKEYS 256
#define NMOUSE 16

unsigned char key_state[NKEYS];
unsigned char key_state_last[NKEYS];
unsigned char mouse_state[NMOUSE];
unsigned char mouse_state_last[NMOUSE];

void set_mouse(int x, int y);
void mouse_update(void);
void idle(void);
void display(void);
void motion(int x, int y);
void passive_motion(int x, int y);
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void keyboard_up(unsigned char key, int x, int y);

int get_key_press(unsigned char key)
{
	return key_state[key] && !key_state_last[key];
}

int get_key(unsigned char key)
{
	return key_state[key];
}

int get_key_release(unsigned char key)
{
	return key_state_last[key] && !key_state[key];
}

int get_mouse_press(int button)
{
	return mouse_state[button] && !mouse_state_last[button];
}

int get_mouse(int button)
{
	return mouse_state[button];
}

int get_mouse_release(int button)
{
	return mouse_state_last[button] && !mouse_state[button];
}


int main(int argc, char *argv[])
{
	int i;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow(window_title);
	glutIdleFunc(idle);
	glutDisplayFunc(display);
        glutMotionFunc(motion);
        glutMouseFunc(mouse);
        glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboard_up);
        glutPassiveMotionFunc(passive_motion);

        glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearDepth(1);

	mouse_x = 0;
	mouse_y = 0;
	window_mouse_x = 0;
	window_mouse_y = 0;
	for (i = 0; i < NKEYS; i++) {
		key_state[i] = 0;
		key_state_last[i] = 0;
	}
	for (i = 0; i < NMOUSE; i++) {
		mouse_state[i] = 0;
		mouse_state_last[i] = 0;
	}

	app_start();
	glutMainLoop();
	app_exit();
	return 0;
}

void idle(void)
{
	static time_t last_frame_time = 0;
	int i;
	if (clock() - last_frame_time >= CLOCKS_PER_SEC / fps) {
		last_frame_time = clock();
		mouse_update();
		app_update();
		mouse_update();
		for (i = 0; i < NKEYS; i++) key_state_last[i] = key_state[i];
		for (i = 0; i < NMOUSE; i++) mouse_state_last[i]=mouse_state[i];
		glutPostRedisplay();
	}
}

void display(void)
{
	window_width = glutGet(GLUT_WINDOW_WIDTH);
	window_height = glutGet(GLUT_WINDOW_HEIGHT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glScaled(2.0f/view_width, -2.0f/view_height, 1);
	glRotated(view_angle, 0, 0, -1);
	glTranslated(-view_x-view_width/2.0f, -view_y - view_height/2.0f, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	app_draw_model();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslated(-1.0f, 1.0f, 0);
	glScaled(2.0f / window_width, -2.0f / window_height, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	app_draw_hud();

	glutSwapBuffers();
	idle();
}

void motion(int x, int y)
{
	set_mouse(x, y);
}

void passive_motion(int x, int y)
{
	set_mouse(x, y);
}

void mouse(int button, int state, int x, int y)
{
	mouse_state[button] = state == GLUT_DOWN;
}

void keyboard(unsigned char key, int x, int y)
{
	key_state[ascii_to_keycode(key)] = 1;
}

void keyboard_up(unsigned char key, int x, int y)
{
	key_state[ascii_to_keycode(key)] = 0;
}

void set_mouse(int x, int y)
{
	window_mouse_x = x;
	window_mouse_y = y;
}

void mouse_update(void)
{
	double rd, xx, yy;
	/* window coords to view coords:
	 * translate	[-view_width / 2, -view_height / 2]
	 * scale	[view_width/window_width, view_height/window_height]
	 * rotate	view_angle
	 * translate	[view_x + view_width / 2, view_y + view_height / 2] */
	rd = view_angle * PI / 180.0f;
	mouse_x = window_mouse_x - window_width / 2.0f;
	mouse_y = window_mouse_y - window_height / 2.0f;
	mouse_x *= (double) view_width / window_width;
	mouse_y *= (double) view_height / window_height;
	xx = mouse_x * cos(rd) - mouse_y * sin(rd);
	yy = mouse_y * cos(rd) + mouse_x * sin(rd);
	mouse_x = xx;
	mouse_y = yy;
	mouse_x += view_x + view_width / 2.0f;
	mouse_y += view_y + view_height / 2.0f;
}
