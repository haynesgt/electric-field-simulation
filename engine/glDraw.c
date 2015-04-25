#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <math.h>
#ifndef PI
#define PI 3.14159265358979323846264338327950288419716939937510582097494459230
#endif
// http://slabode.exofire.net/circle_draw.shtml
// credit to "siegelords": siegelords_abode@yahoo.com

void glDrawCircle(double cx, double cy, double r, int num_segments, int fill) 
{ 
	int ii;
	double theta, x, y;
	if (fill) {
		glBegin(GL_TRIANGLE_FAN);
		glVertex2d(cx, cy);
	} else {
		glBegin(GL_LINE_LOOP); 
	}
	theta = 0;
	for(ii = 0; ii < num_segments; ii++) 
	{ 
		//get the current angle 
		theta += 2.0f * PI / num_segments;

		x = r * cosf(theta);//calculate the x component 
		y = r * sinf(theta);//calculate the y component 

		glVertex2d(x + cx, y + cy);//output vertex 
	} 
	if (fill) glVertex2d(cx + r, cy);
	glEnd(); 
}
#if 0
{ 
	double theta = 2 * 3.1415926 / (double)(num_segments); 
	double tangetial_factor = tanf(theta);//calculate the tangential factor 

	double radial_factor = cosf(theta);//calculate the radial factor 
	
	double x = r;//we start at angle = 0 

	double y = 0; 
	int ii;
    
	glBegin(GL_LINE_LOOP); 
	for(ii = 0; ii < num_segments; ii++) 
	{ 
		glVertex2d(x + cx, y + cy);//output vertex 
        
		//calculate the tangential vector 
		//remember, the radial vector is (x, y) 
		//to get the tangential vector we flip those coordinates and negate one of them 

		double tx = -y; 
		double ty = x; 
        
		//add the tangential vector 

		x += tx * tangetial_factor; 
		y += ty * tangetial_factor; 
        
		//correct using the radial factor 

		x *= radial_factor; 
		y *= radial_factor; 
	} 
	glEnd(); 
}
#endif

void glDrawArc(double cx, double cy, double r, double start_angle, double arc_angle, int num_segments) 
{ 
	double theta, tangetial_factor, radial_factor, x, y, tx, ty;
	int ii;
	// theta is now calculated from the arc angle instead, the (-1)
	// comes from the fact that the arc is open
	theta = arc_angle / (double)(num_segments - 1);
	tangetial_factor = tanf(theta);
	radial_factor = cosf(theta);
	x = r * cosf(start_angle);//we now start at the start angle
	y = r * sinf(start_angle); 
    
	glBegin(GL_LINE_STRIP);
	for(ii = 0; ii < num_segments; ii++)
	{ 
		glVertex2d(x + cx, y + cy);

		tx = -y; 
		ty = x; 

		x += tx * tangetial_factor; 
		y += ty * tangetial_factor; 

		x *= radial_factor; 
		y *= radial_factor; 
	} 
	glEnd(); 
}
