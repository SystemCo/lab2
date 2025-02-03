//
//modified by: Francisco Andrade
//date: 02/01/25
//
//original author: Gordon Griesel
//date:            2025
//purpose:         OpenGL sample program
//
//This program needs some refactoring.
//We will do this in class together.
//
// to do list
// 1/31/2025 add some text
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

//some structures

class Global {
public:
	int xres, yres;
	float w;
	float dir[2];
	float pos[2];
	Global()
    {
	    xres = 400;
	    yres = 200;
	    w = 20.0f;
	    dir[0] = 32.0f;
        dir[1] = 28.0f;
	    pos[0] = 0.0f + w;
        pos[1] = yres/2.0f;
    }
} g;

class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	~X11_wrapper();
	X11_wrapper();
	void set_title();
	bool getXPending();
	XEvent getXNextEvent();
	void swapBuffers();
	void reshape_window(int width, int height);
	void check_resize(XEvent *e);
	void check_mouse(XEvent *e);
	int check_keys(XEvent *e);
} x11;

//Function prototypes
void init_opengl(void);
void physics(void);
void render(void);


int main()
{
	init_opengl();
	int done = 0;
	//main game loop
	while (!done) {
		//look for external events such as keyboard, mouse.
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			x11.check_mouse(&e);
			done = x11.check_keys(&e);
		}
		physics();
		render();
		x11.swapBuffers();
		usleep(200);
	}
	cleanup_fonts();
	return 0;
}

X11_wrapper::~X11_wrapper()
{
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper()
{
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w = g.xres, h = g.yres;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		cout << "\n\tcannot connect to X server\n" << endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		cout << "\n\tno appropriate visual found\n" << endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask =
		ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title()
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "3350 Lab-2 - Esc to exit");
}

bool X11_wrapper::getXPending()
{
	//See if there are pending events.
	return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent()
{
	//Get a pending event.
	XEvent e;
	XNextEvent(dpy, &e);
	return e;
}

void X11_wrapper::swapBuffers()
{
	glXSwapBuffers(dpy, win);
}

void X11_wrapper::reshape_window(int width, int height)
{
	//Window has been resized.
	g.xres = width;
	g.yres = height;
	//
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
}

void X11_wrapper::check_resize(XEvent *e)
{
	//The ConfigureNotify is sent by the
	//server if the window is resized.
	if (e->type != ConfigureNotify)
		return;
	XConfigureEvent xce = e->xconfigure;
	if (xce.width != g.xres || xce.height != g.yres) {
		//Window size did change.
		reshape_window(xce.width, xce.height);
	}
}
//-----------------------------------------------------------------------------

void X11_wrapper::check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;

	//Weed out non-mouse events
	if (e->type != ButtonRelease &&
		e->type != ButtonPress &&
		e->type != MotionNotify) {
		//This is not a mouse event that we care about.
		return;
	}
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed.
			//int y = g.yres - e->xbutton.y;
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed.
			return;
		}
	}
	if (e->type == MotionNotify) {
		//The mouse moved!
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			savex = e->xbutton.x;
			savey = e->xbutton.y;
			//Code placed here will execute whenever the mouse moves.


		}
	}
}

int X11_wrapper::check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_f:
				//the 'f' key was pressed
				if(static_cast<int>(abs(g.dir[0])) < 100) {
                    g.dir[0] *= 2.0;
                    g.dir[1] *= 2.0;
                }
                break;
            case XK_s:
                //the 's' key was pressed
                if(static_cast<int>(abs(g.dir[1])) > 10) {
                    g.dir[0] /= 2.0;
                    g.dir[1] /= 2.0;
                }
                break;
			case XK_Escape:
				//Escape key was pressed
				return 1;
		}
	}
	return 0;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, g.xres, g.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	
	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void physics()
{
	// Move the box
    g.pos[0] += g.dir[0];
    g.pos[1] += g.dir[1];
    // Collision detection
	//Wall collision detection
    if (g.pos[0] >= (g.xres-g.w)) {
		g.pos[0] = (g.xres-g.w);
		g.dir[0] = -g.dir[0];
	}
	if (g.pos[0] <= g.w) {
		g.pos[0] = g.w;
		g.dir[0] = -g.dir[0];
	}
    //Ceiling & floor collision detection
	if (g.pos[1] >= (g.yres-g.w)) {
		g.pos[1] = (g.yres-g.w);
		g.dir[1] = -g.dir[1];
	}
	if (g.pos[1] <= g.w) {
		g.pos[1] = g.w;
		g.dir[1] = -g.dir[1];
	}
}

void render()
{
	//clear the window
	glClear(GL_COLOR_BUFFER_BIT);
	//draw the box if the window width is larger than the box width
	if(g.xres >= (g.w+20) && g.yres >= (g.w+20)) {
        glPushMatrix();
        //glColor3ub(100, 120, 220);
        int red, green, blue;
        const int X_WINDOW_SIZE = 1080;
        red = 255 - 255*g.xres / X_WINDOW_SIZE;
        if(red < 0)
            red = 0;
        blue = 255*g.xres / X_WINDOW_SIZE;
        if(blue > 255)
            blue = 255;
        green = (red <= blue) ? 2*red : 2*blue;
        glColor3ub(red, green, blue);
        // For debugging purposes
        cout << red << " " << blue << " " << green << " "; 
        cout << g.xres << " " << g.yres << " ";
        cout << g.dir[0] << " " << g.dir[1] << endl;
    }
    glTranslatef(g.pos[0], g.pos[1], 0.0f);
	glBegin(GL_QUADS);
		glVertex2f(-g.w, -g.w);
		glVertex2f(-g.w,  g.w);
		glVertex2f( g.w,  g.w);
		glVertex2f( g.w, -g.w);
	glEnd();
	glPopMatrix();
	
	Rect r;
    	//
    	r.bot = g.yres - 20;
    	r.left = 10;
    	r.center = 0;
    	ggprint8b(&r, 16, 0x00ff0000, "3350 - lab-2");
    	ggprint8b(&r, 16, 0x00ffff00, "Esc to exit");
    	ggprint8b(&r, 16, 0x00ffff00, "F   speed up");
    	ggprint8b(&r, 16, 0x00ffff00, "S   slow down");
}

