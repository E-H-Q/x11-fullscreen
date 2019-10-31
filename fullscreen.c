#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <string.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

Display *dis;
Window win;
XEvent report;
XButtonEvent *xb = (XButtonEvent *)&report;
GC white_gc;
GC green_gc;
GC black_gc;
XColor white_col;
XColor green_col;
XColor black_col;
Colormap colormap;

char white[] = "#FFFFFF";
char green[] = "#00FF00";
char black[] = "#000000";

int x = 0;
int y = 0;
int i;
int btn = 0; /* Determines which is selected: 0 = windowed & 1 = fullscreen */
int width = 550; /* sets X size of window */
int height = 550; /* sets Y size of window */
int height_orig = 550;
int width_orig = 550;
int w;
int h;

typedef struct { /* Magic struct for window border/decoration removal */
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long inputMode;
	unsigned long status;
} Hints;

Hints hints;
Atom property;

void fill() {
	if (btn == 0) {
		for (i = 0; i < 50; i++) {
			XDrawRectangle(dis, win, green_gc, width/2+50, height/2-25, 100, 50-i);
			XDrawString (dis, win, black_gc, width/2+75, height/2+3, "WINDOWED", 8);
		}
	}
	else if (btn == 1) {
		for (i = 0; i < 50; i++) {
			XDrawRectangle(dis, win, green_gc, width/2-150, height/2-25, 100, 50-i);
			XDrawString (dis, win, black_gc, width/2-130, height/2+3, "FULLSCREEN", 10);
		}
	}
}

void redraw() {
	XClearWindow(dis, win);
	XDrawString (dis, win, white_gc, 5, height-10, "Press ESC to exit", 17);
	XDrawRectangle(dis, win, green_gc, width/2-150, height/2-25, 100, 50);
	XDrawString (dis, win, white_gc, width/2-130, height/2+3, "FULLSCREEN", 10);
	XDrawRectangle(dis, win, green_gc, width/2+50, height/2-25, 100, 50);
	XDrawString (dis, win, white_gc, width/2+75, height/2+3, "WINDOWED", 8);
	fill();
}

int getScreenSize(int *w, int*h) {
	Screen* screen = NULL;

	if (!dis) {
		fprintf(stderr, "Failed to open default display.\n");
		return -1;
	}
	screen = DefaultScreenOfDisplay(dis);
	if ( !screen ) {
		fprintf(stderr, "Failed to obtain the default screen of given display.\n");
		return -2;
	}

	*w = screen->width;
 	*h = screen->height;

	return 0;
}

int main() {
	dis = XOpenDisplay(NULL);
	win = XCreateSimpleWindow(dis, RootWindow(dis, 0), 1, 1,
		width, /* X window size */
		height, /* Y window size */
		0, BlackPixel (dis, 0), BlackPixel(dis, 0));
	XMapWindow(dis, win);
	XSetStandardProperties(dis, win, "fullscreen.c", "", None, NULL, 0, NULL);
	XWindowAttributes xwAttr;

	colormap = DefaultColormap(dis, 0);
	white_gc = XCreateGC(dis, win, 0, 0);
	XParseColor(dis, colormap, white, &white_col);
	XAllocColor(dis, colormap, &white_col);
	XSetForeground(dis, white_gc, white_col.pixel);
	green_gc = XCreateGC(dis, win, 0, 0);
	XParseColor(dis, colormap, green, &green_col);
	XAllocColor(dis, colormap, &green_col);
	XSetForeground(dis, green_gc, green_col.pixel);
	black_gc = XCreateGC(dis, win, 0, 0);
	XParseColor(dis, colormap, black, &black_col);
	XAllocColor(dis, colormap, &black_col);
	XSetForeground(dis, black_gc, black_col.pixel);

	XSelectInput(dis, win, ExposureMask | KeyPressMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask);
	XFlush(dis);
	XMapWindow (dis, win);

	redraw();

	getScreenSize(&w, &h);
	/* printf ("Screen: width = %d, height = %d \n", w, h); */

	while (1) {
		XNextEvent(dis, &report);
		if (report.type == Expose) {
			redraw();
		}
		if (report.type == ConfigureNotify) { /* detect window size change(s) */
			XConfigureEvent xce = report.xconfigure;
			if (xce.width != width || xce.height != height) {
				width = xce.width;
				height = xce.height;
			}
			redraw();
		}
		if (report.type == KeyPress) {
			if (XLookupKeysym(&report.xkey, 0) == XK_Escape) {
				exit(0); /* exits the program */
			} 
		}
		if (report.type == ButtonPress) {
			x = report.xbutton.x;
			y = report.xbutton.y;
			if (report.xbutton.button == Button1) { /* left click */
				if ((x > width/2+50) && (x < width/2+150) && (y > height/2-25) && (y < height/2+25)) {
					btn = 0; /* Windowed */
					XMoveResizeWindow(dis, win, w/2-width_orig/2, h/2-height_orig/2, width_orig, height_orig); /* moves to center, and restores original size */
					redraw();
				}
				else if ((x > width/2-150) && (x < width/2-50) && (y > height/2-25) && (y < height/2+25)) {
					btn = 1; /* Fullscreen */
					XMoveResizeWindow(dis, win, 0, 0, w, h); /* moves to (0,0) and resizes to fit screen */
					hints.flags = 2;
					hints.decorations = 1;
					property = XInternAtom(dis, "_MOTIF_WM_HINTS", True);
					XChangeProperty(dis, win, property, property, 32, PropModeReplace, (unsigned char *)&hints, 5);
					redraw();
				}
			}
		}
	}
	return 0;
}
