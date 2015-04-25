#include "global.h"
#include "glDraw.h"
#define GL_GLEXT_PROTOTYPES
#ifdef _WIN32
#include <windows.h>
#define USE_FRAMEBUFFER 0
#else
#define USE_FRAMEBUFFER 0
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <math.h>
#include "png_texture_load.h"

#define VIEW_W 512
#define VIEW_H 512

const char* window_title = "Demo";
double view_x = 0;
double view_y = 0;
double view_width = VIEW_W;
double view_height = VIEW_H;
double view_angle = 0;
int window_width = VIEW_W;
int window_height = VIEW_H;
double fps = 60;

struct charge_t {
	double x, y, q;
	struct charge_t* next;
	struct charge_t* prev;
};

struct charge_t* charge0;
struct charge_t* charge_sel; // selected charge

#define BALLN 64
struct ball_t {
	double x, y, vx, vy;
} ball[BALLN];

double frac(double);
void draw_test_lines_and_text(void);

int show_force_field;
int show_field_lines;
int show_test_data;
int show_balls;

#if USE_FRAMEBUFFER
#define IMG_W VIEW_W*2
#define IMG_H VIEW_H*2
#else
#define IMG_W VIEW_W
#define IMG_H VIEW_H
#endif
double field_pe[IMG_W*IMG_H];
double field_fx[IMG_W*IMG_H];
double field_fy[IMG_W*IMG_H];
unsigned int image_w = IMG_W, image_h = IMG_H;
unsigned int image_tex;
unsigned int image_line_tex;
#if USE_FRAMEBUFFER
unsigned int image_fb, buffer_tex;
#endif

void image_clear(void);
void image_init(void);
void image_redraw_field_lines(void);
void image_new_charge(double x, double y, double q);
void image_draw_force_field(void);
void image_draw_field_lines(void);

void field_init(void);
void field_trace_field_line(double, double, double, unsigned int);
void field_trace_equipotential_line(double, double, double, unsigned int);
double field_get_strength(double x, double y, double* fx, double* fy);
double field_get_voltage(double x, double y);
struct charge_t* field_add_charge(double x, double y, double q);
void field_remove_charge(struct charge_t* charge);
void field_reset(void);
struct charge_t* field_get_nearest_charge(double x, double y);

void ex0(void); // Two opposite charges against each other
void ex1(void); // Two instances of ex0
void ex2(void); // ex0 but with aligned with another charge
void ex3(void); // A perfect square, with opposite charges on adjacent corners
void ex4(void); // A row of equal charges
void ex5(void); // A circlular pattern of alternating charges
void ex6(void); // A circlular pattern of the same charge

double frac(double v)
{
	return v - floor(v);
}

void app_start(void)
{
	int i;
	glClearColor(0, 0, 0, 0);
	srand((unsigned int)time(NULL));
	charge_sel = NULL;
	field_init();
	image_init();
	for (i = 0; i < BALLN; i++) {
		ball[i].vx = 0;
		ball[i].vy = 0;
	}
	show_force_field = 1;
	show_field_lines = 1;
	show_test_data = 1;
	show_balls = 1;
	printf( "Charges are either 1C or -1C\n"
		"Each pixel represents one metre."
		"The whole window is %d*%d\n"
		"Contour lines are 5x10^7 volts apart\n"
		"Press number keys 1-4 to show/hide various features\n"
		"Press z/x/c/v/b/n/m to see example set-ups\n"
		"Press space to reset the program\n"
		"Created by Gavin Haynes 2013\n", VIEW_W, VIEW_H);
}

void app_exit(void)
{
	field_reset();
	glDeleteTextures(1, &image_tex);
	glDeleteTextures(1, &image_line_tex);
#if USE_FRAMEBUFFER
	glDeleteTextures(1, &buffer_tex);
	glDeleteFramebuffersEXT(1, &image_fb);
#endif
}

void app_update(void)
{
	int i;
	double fx, fy;
	if (get_mouse_press(0)) field_add_charge(mouse_x, mouse_y, 1.0);
	if (get_mouse_press(1) && charge_sel != NULL) 
			field_remove_charge(charge_sel);
	if (get_mouse_press(2)) field_add_charge(mouse_x, mouse_y, -1.0);
	if (get_key_press('1')) show_force_field = 1 - show_force_field;
	if (get_key_press('2')) show_field_lines = 1 - show_field_lines;
	if (get_key_press('3')) show_test_data = 1 - show_test_data;
	if (get_key_press('4')) show_balls = 1 - show_balls;
	if (get_key_press(' ')) field_reset();
	if (get_key_press('Z')) ex0();
	if (get_key_press('X')) ex1();
	if (get_key_press('C')) ex2();
	if (get_key_press('V')) ex3();
	if (get_key_press('B')) ex4();
	if (get_key_press('N')) ex5();
	if (get_key_press('M')) ex6();
	if (show_balls) {
		for (i = 0; i < BALLN; i++) {
			if (ball[i].x <= 0 || ball[i].x > view_width || 
						ball[i].y < 0 || 
						ball[i].y > view_height) {
				ball[i].x = rand() % (int)view_width;
				ball[i].y = rand() % (int)view_height;
				ball[i].vx = 0;
				ball[i].vy = 0;
			}
			field_get_strength(ball[i].x, ball[i].y, &fx, &fy);
			ball[i].x += ball[i].vx * 3e-4 + 0.5 * fx * 9e-8;
			ball[i].y += ball[i].vy * 3e-4 + 0.5 * fy * 9e-8;
			ball[i].vx += fx * 3e-4;
			ball[i].vy += fy * 3e-4;
		}
	}
//	view_width = window_width;
//	view_height = window_height;
	if (window_width < image_w) glutReshapeWindow(image_w, window_height);
	if (window_height < image_h) glutReshapeWindow(window_width, image_h);
}

void app_draw_model(void)
{
	int i;
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (show_force_field) image_draw_force_field();
	if (show_field_lines) image_draw_field_lines();
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	glColor4d(1, 1, 1, 1);
	if (show_balls) {
		for (i = 0; i < BALLN; i++) {
			glDrawCircle(ball[i].x, ball[i].y, 2, 8, 1);
		}
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	charge_sel = field_get_nearest_charge(mouse_x, mouse_y);
	if (show_test_data) draw_test_lines_and_text();
}

void app_draw_hud(void)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void draw_test_lines_and_text(void)
{
	// Draw a field curve and an equipotential curve at the mouse location
	// and report the potential E and force at the mouse location
	char txt[512];
	double mouse_e, mouse_ex, mouse_ey, mouse_f; // e=energy f=field
	if (charge_sel != NULL) {
		glColor4d(1.0, 1.0, 1.0, 0.2);
		glDrawCircle(charge_sel->x, charge_sel->y, 
							charge_sel->q*8, 8, 1);
	}
	mouse_e = field_get_voltage(mouse_x, mouse_y);
	field_get_strength(mouse_x, mouse_y, &mouse_ex, &mouse_ey);
	mouse_f = sqrt(mouse_ex*mouse_ex + mouse_ey*mouse_ey);
	sprintf(txt, "%.3eV, %.3eN/C", mouse_e, mouse_f);
	glColor4d(0, 0, 0, 1);
	glRasterPos2i((int)mouse_x+1, (int)mouse_y+1);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (unsigned char*)txt);
	glColor4d(1, 1, 1, 1);
	glRasterPos2i((int)mouse_x, (int)mouse_y);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (unsigned char*)txt);
	glColor4d(1, 1, 1, 1);
	field_trace_field_line(mouse_x, mouse_y, 4, 30000);
	field_trace_field_line(mouse_x, mouse_y, -4, 10000);
	glColor4d(0, 0, 0, 1);
	field_trace_equipotential_line(mouse_x, mouse_y, 0.1, 10000);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ex0(void)
{
	field_reset();
	field_add_charge(view_width / 2, view_height / 2 - 4, 1);
	field_add_charge(view_width / 2, view_height / 2 + 4, -1);
}

void ex1(void)
{
	field_reset();
	field_add_charge(12, 16, 1);
	field_add_charge(16, 16, -1);
	field_add_charge(view_width - 16, view_height - 16, 1);
	field_add_charge(view_width - 12, view_height - 16, -1);
}

void ex2(void)
{
	field_reset();
	field_add_charge(60, view_height / 2, -1);
	field_add_charge(64, view_height / 2, 1);
	field_add_charge(view_width - 64, view_height / 2, -1);
}

void ex3(void)
{
	field_reset();
	field_add_charge(view_width * 0.25, view_height * 0.25, 1);
	field_add_charge(view_width * 0.75, view_height * 0.25, -1);
	field_add_charge(view_width * 0.25, view_height * 0.75, -1);
	field_add_charge(view_width * 0.75, view_height * 0.75, 1);
}

void ex4(void)
{
	double x;
	field_reset();
	for (x = view_width / 2 - 50; x < view_width / 2 + 50; x += 25) {
		field_add_charge(x, view_height * 0.4, 1);
		field_add_charge(x, view_height * 0.6, -1);
	}
}

void ex5(void)
{
	double a, xx, yy, c, r;
	field_reset();
	c = 1;
	r = view_width / 4;
	for (a = 0; a < 2 * PI; a += PI / 8) {
		xx = r * cos(a);
		yy = r * sin(a);
		field_add_charge(view_width / 2 + xx, view_height / 2 + yy, c);
		c *= -1;
	}
}

void ex6(void)
{
	double a, xx, yy, r;
	field_reset();
	r = view_width / 4;
	for (a = 0; a < 2 * PI; a += PI / 8) {
		xx = r * cos(a);
		yy = r * sin(a);
		field_add_charge(view_width / 2 + xx, view_height / 2 + yy, 1);
	}
}

/* GRID ************************************************************************
 *
 * image_clear, image_redraw_field_lines, image_new_charge, and image_draw
 *
 ******************************************************************************/

void image_clear(void)
{
	unsigned int i;
	for (i = 0; i < image_w * image_h; i++) {
		field_pe[i] = 0;
		field_fx[i] = 0;
		field_fy[i] = 0;
	}

#if USE_FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, image_fb);
#endif
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture (GL_TEXTURE_2D, image_tex);
	glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, image_w, image_h,0);
	glBindTexture (GL_TEXTURE_2D, image_line_tex);
	glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, image_w, image_h,0);
#if USE_FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

void image_init(void)
{
	glGenTextures(1, &image_tex);
	glBindTexture(GL_TEXTURE_2D, image_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image_w, image_h, 0, GL_RGBA,
						GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &image_line_tex);
	glBindTexture(GL_TEXTURE_2D, image_line_tex);
#if USE_RENDERBUFFER
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image_w/2, image_h/2, 
					0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image_w, image_h, 
					0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#if USE_FRAMEBUFFER
	glGenTextures(1, &buffer_tex);
	glBindTexture(GL_TEXTURE_2D, buffer_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image_w, image_h, 0, GL_RGBA,
						GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &image_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, image_fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
						GL_TEXTURE_2D, buffer_tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

	image_clear();
}

void image_redraw_field_lines(void)
{
	struct charge_t* charge_it;
	double a, s, fx, fy;
	int viewport[4];
        glGetIntegerv(GL_VIEWPORT, (int*) viewport);
#if USE_FRAMEBUFFER
	glViewport(0, 0, image_w/2, image_h/2);
	glBindFramebuffer(GL_FRAMEBUFFER, image_fb);
#else
	glViewport(0, 0, image_w, image_h);
#endif
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	// glScaled(image_w, image_h, 1);
	// glScaled(image_w / view_width, image_h / view_height, 1);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor4d(1, 1, 1, 1);
	for (charge_it = charge0; charge_it != 0; charge_it = charge_it->next) {
		field_get_strength(charge_it->x, charge_it->y, &fx, &fy);
		s = atan2(-fy, fx);
		for (a = 0; a < 2 * PI; a += PI / 4) {
			if (charge_it->q > 0) 	field_trace_field_line(
				charge_it->x + 8 * cos(a + s), 
				charge_it->y - 8 * sin(a + s), 2, 400);
			else field_trace_field_line(
				charge_it->x + 8 * cos(a + s), 
				charge_it->y - 8 * sin(a + s), -2, 400);

		}
	}
	glBindTexture (GL_TEXTURE_2D, image_line_tex);
#if USE_FRAMEBUFFER
	glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 
					image_w/2, image_h/2, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#else
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, image_w, image_h,0);
#endif
	glPopMatrix();
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void image_new_charge(double x, double y, double q)
{
	// updates the image to show the new force field
	unsigned int xi, yi;
	double xx, yy, dx, dy, d, e, l, cw, ch;
	int viewport[4];
        glGetIntegerv(GL_VIEWPORT, (int*) viewport);
	glViewport(0, 0, image_w, image_h);
#if USE_FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, image_fb);
#endif

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, image_w, image_h, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cw = 1; // view_width / image_w;
	ch = 1; // view_height / image_h;
	glBegin(GL_POINTS);
	for (xi = 0; xi < image_w; xi++) {
		for (yi = 0; yi < image_h; yi++) {
			xx = xi * cw;
			yy = yi * ch;
			dx = xx - x;
			dy = yy - y;
			d = sqrt(dx*dx + dy*dy);
			if (d != 0) {
				field_pe[yi*image_w+xi] += 8.99e9f * q/d;
				field_fx[yi*image_w+xi] += 8.99e8f*q/d/d*dx/d;
				field_fy[yi*image_w+xi] += 8.99e8f*q/d/d*dy/d;
			}
			e = field_pe[yi*image_w + xi];
			l = e / 5e7;
			if (e > 1e-6) glColor4d(1, frac(l)*0.75, 0, 1);
			else if (e < -1e-6) glColor4d(0, frac(l)*0.75, 1, 1);
			else glColor4d(0, 0, 0, 1);
			glVertex2i(xi, yi);
		}
	}
	glEnd();
	glBindTexture (GL_TEXTURE_2D, image_tex);
	glCopyTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, image_w, image_h,0);
	glPopMatrix();
#if USE_FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	if (show_field_lines) image_redraw_field_lines();
}

void image_draw_force_field(void)
{
	glEnable(GL_TEXTURE_2D);
	glColor4d(1, 1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, image_tex);
	glBegin(GL_QUADS);
	glTexCoord2f (0, 1);
	glVertex2d(0, 0);
	glTexCoord2f (1, 1);
	glVertex2d(image_w, 0);
	glTexCoord2f (1, 0);
	glVertex2d(image_w, image_h);
	glTexCoord2f (0, 0);
	glVertex2d(0, image_h);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}
void image_draw_field_lines(void)
{
	glColor4d(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, image_line_tex);
	glBegin(GL_QUADS);
	glTexCoord2f (0, 1);
	glVertex2d(0, 0);
	glTexCoord2f (1, 1);
	glVertex2d(image_w, 0);
	glTexCoord2f (1, 0);
	glVertex2d(image_w, image_h);
	glTexCoord2f (0, 0);
	glVertex2d(0, image_h);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

/* FIELD ***********************************************************************
 *
 * field_trace_field_line, field_trace_equipotential_line, field_get_strength,
 * field_add_charge, and field_remove_charge
 *
 ******************************************************************************/

void field_init(void)
{
	charge0 = NULL;
}

void field_trace_field_line(double x, double y, double r, unsigned int len)
{
	unsigned int i;
	double xx, yy, fx, fy, f;
	double col[4];
	double def_alpha;
	glGetDoublev(GL_CURRENT_COLOR, col);
	def_alpha = col[3];
	xx = x;
	yy = y;
	glBegin(GL_LINE_STRIP);
	for (i = 0; i < len; i++) {
		col[3] = frac(i * r / 16.0) + 0.5;
		glColor4dv(col);
		glVertex2d(xx, yy);
		f = field_get_strength(xx, yy, &fx, &fy);
		if (f != 0) {
			xx += r * fx / f;
			yy += r * fy / f;
		} else break;
	}
	glEnd();
	col[3] = def_alpha;
	glColor4dv(col);
}

void field_trace_equipotential_line(double x, double y, double r, 
							unsigned int len)
{
	unsigned int i;
	double xx, yy, fx, fy, f;
	len /= 2;
	xx = x;
	yy = y;
	glBegin(GL_LINE_STRIP);
	for (i = 0; i < len; i++) {
		glVertex2d(xx, yy);
		f = field_get_strength(xx, yy, &fx, &fy);
		if (f != 0) {
			xx -= r * fy / f;
			yy += r * fx / f;
		} else break;
	}
	glEnd();
	xx = x;
	yy = y;
	glBegin(GL_LINE_STRIP);
	for (i = 0; i < len; i++) {
		glVertex2d(xx, yy);
		f = field_get_strength(xx, yy, &fx, &fy);
		if (f != 0) {
			xx += r * fy / f;
			yy -= r * fx / f;
		} else break;
	}
	glEnd();
}

double field_get_strength(double x, double y, double* fx, double* fy)
{
#if 0
	unsigned int n;
	if (x >= 0 && y >= 0 && x < view_width && y < view_height) {
		x *= image_w / view_width;
		y *= image_h / view_height;
		n = (unsigned int)(floor(y) * image_w + floor(x));
		*fx = field_fx[n];
		*fy = field_fy[n];
	} else {
		*fx = 0;
		*fy = 0;
	}
#else
	struct charge_t* charge_it;
	double dx, dy, d;
	*fx = 0;
	*fy = 0;
	for (charge_it = charge0; charge_it != 0; charge_it = charge_it->next) {
		dx = x - charge_it->x;
		dy = y - charge_it->y;
		d = sqrt(dx*dx + dy*dy);
		if (d != 0) {
			*fx += 8.99e9 * charge_it->q / d / d * dx / d;
			*fy += 8.99e9 * charge_it->q / d / d * dy / d;
		}
	}
#endif
	return (sqrt((*fx)*(*fx) + (*fy)*(*fy)));
}

double field_get_voltage(double x, double y)
{
	struct charge_t* charge_it;
	double dx, dy, d, e;
	e = 0;
	for (charge_it = charge0; charge_it != 0; charge_it = charge_it->next) {
		dx = x - charge_it->x;
		dy = y - charge_it->y;
		d = sqrt(dx*dx + dy*dy);
		if (d != 0) {
			e += 8.99e9 * charge_it->q / d;
		}
	}
	return e;
}

struct charge_t* field_add_charge(double x, double y, double q)
{
	struct charge_t* new_charge;
	new_charge = malloc(sizeof(struct charge_t));
	new_charge->x = x;
	new_charge->y = y;
	new_charge->q = q;
	new_charge->next = charge0;
	new_charge->prev = NULL;
	if (charge0 != NULL) charge0->prev = new_charge;
	charge0 = new_charge;
	image_new_charge(x, y, q);
	charge_sel = new_charge;
	return new_charge;
}

void field_remove_charge(struct charge_t* charge)
{
	if (charge->prev != NULL) charge->prev->next = charge->next;
	if (charge->next != NULL) charge->next->prev = charge->prev;
	if (charge == charge0) charge0 = charge0->next;
	image_new_charge(charge->x, charge->y, 0-charge->q);
	free(charge);
	if (charge0 == NULL) image_clear();
	if (charge == charge_sel) charge_sel = NULL;
}

void field_reset(void)
{
	struct charge_t* it;
	while (charge0 != NULL) {
		it = charge0->next;
		free(charge0);
		charge0 = it;
	}
	image_clear();
}

struct charge_t* field_get_nearest_charge(double x, double y)
{
	struct charge_t *charge_it, *sel;
	double min_d, d, dx, dy;
	min_d = 1e10;
	sel = NULL;
	for (charge_it = charge0; charge_it != 0; charge_it = charge_it->next) {
		dx = x - charge_it->x;
		dy = y - charge_it->y;
		d = sqrt(dx*dx + dy*dy);
		if (d < min_d) {
			min_d = d;
			sel = charge_it;
		}
	}
	return sel;
}
