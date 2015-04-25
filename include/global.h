#ifndef GLOBAL_H_INCLUDE
#define GLOBAL_H_INCLUDE
#ifndef PI
#define PI 3.14159265358979323846264338327950288419716939937510582097494459230
#endif
const char* window_title;
double mouse_x;
double mouse_y;
int window_mouse_x;
int window_mouse_y;
double view_x;
double view_y;
double view_width;
double view_height;
double view_angle;
int window_width;
int window_height;
double fps;

/*
 * For the following functions, the keys are either capital letters, 
 * symbols (without shift: ',.' not '<>'), or GLUT macros (GLUT_KEY_LEFT, etc.).
 */
int get_key_press(unsigned char key);
int get_key(unsigned char key);
int get_key_release(unsigned char key);
/*
 * 1 = Left 2 = middle 3 = right. Buttons 3 and 4 are the scroll wheel 
 * directions (system dependant).
 */
int get_mouse_press(int button);
int get_mouse(int button);
int get_mouse_release(int button);
#endif
