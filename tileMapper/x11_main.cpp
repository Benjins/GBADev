#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

enum KeyState {
	OFF = 0,
	RELEASE = 1,
	PRESS = 2,
	HOLD = 3
};


extern KeyState keyStates[256];

extern int currMouseX;
extern int currMouseY;
extern int mouseState;

void RunFrame();

void InitText(const char* fileName, int size);

void Init();

extern void* bitmapData;

extern const char* arg1Str;
extern int arg1Length;

Display* display;
int s;

extern int frameWidth;
extern int frameHeight;

GC gc = 0;

int depth = 24; // works fine with depth = 24
int bitmap_pad = 32; // 32 for 24 and 32 bpp, 16, for 15&16
int bytes_per_line = 0; // number of bytes in the client image between the start of one scanline and the start of the next

Pixmap pixmap=0;

 Window window = 0;

XImage* img = NULL;

void Resize(int _w, int _h){
	if(_w != frameWidth || _h != frameHeight){
		frameWidth = _w;
		frameHeight = _h;
	
		free(bitmapData);
		bitmapData = malloc(frameWidth*frameHeight*32);
		
		if(img != NULL){
			//XDestroyImage(img);
		}
			
		img = XCreateImage(
		        display, CopyFromParent,
		        depth, ZPixmap,
		        0, (char *)bitmapData,
		        frameWidth, frameHeight,
		        bitmap_pad, bytes_per_line);

		if(pixmap != 0){
			XFreePixmap(display, pixmap);
		}	
	
		pixmap = XCreatePixmap(display, window, frameWidth, frameHeight, depth);
		
		if(gc != 0){
			XFreeGC(display, gc);
    	}
    	
    	XGCValues gcvalues;
		gc = XCreateGC(display, window, 0, &gcvalues); 
    }
}

Bool evtPred(Display* d, XEvent* evt, XPointer xp){
	return 1;
}

unsigned char KeyCodeToAscii(Display* display, int keycode){
	KeySym ksym = XkbKeycodeToKeysym( display, keycode, 0, keycode & ShiftMask ? 1 : 0);
	
	printf("'%s' was pressed\n", XKeysymToString(ksym));
	
	return XKeysymToString(ksym)[0] & ~('a' ^ 'A');
}

int main(int argc, char **argv)
{
    arg1Str = argv[1];
    arg1Length = 0;
    
    frameWidth = 0;
    frameHeight = 0;
    
    bitmapData = NULL;
    
    for(const char* cursor = argv[1]; cursor && *cursor; cursor++){
    	arg1Length++;
    }
    
    display = XOpenDisplay(0);
    
    s = DefaultScreen(display);
    
    Window root = RootWindow(display, s);
    
    window = XCreateSimpleWindow(display, root, 0, 0, 1280, 700, 1, BlackPixel(display, s), WhitePixel(display, s));
    
    Resize(1280, 720);
          
    XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);           
          
    XMapWindow(display, window);
    
    Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", 0); 
    XSetWMProtocols(display, window, &WM_DELETE_WINDOW, 1);
    
    XEvent ev;
    
    int xOff=0;
    
    InitText("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 18);
    
    Init();
    
    int isRunning = 1;
    
    while (isRunning) {
    	while(XCheckIfEvent(display, &ev, evtPred, NULL)){
			if (ev.type == Expose){
		   		XWindowAttributes windowAttr;
		   		
		   		XGetWindowAttributes(display, window, &windowAttr);
		   		
		   		Resize(windowAttr.width, windowAttr.height);
				 
		    }
		    else if(ev.type == ButtonPress){
		    	if(ev.xbutton.button == 1){
		    		mouseState = 2;
		    	}
		    }
		    else if(ev.type == ButtonRelease){
		    	if(ev.xbutton.button == 1){
		    		mouseState = 1;
		    	}
		    }
		    else if(ev.type == MotionNotify){
		    	currMouseX = ev.xmotion.x;
		    	currMouseY = ev.xmotion.y;
		    }
		    else if (ev.type == KeyPress){
		    	unsigned char key = KeyCodeToAscii(display, ev.xkey.keycode);
		    	keyStates[key] = PRESS;
		    }
		    else if (ev.type == KeyRelease){
		    	unsigned char key = KeyCodeToAscii(display, ev.xkey.keycode);
		    	keyStates[key] = RELEASE;
		    }
		    else if (ev.type == ConfigureNotify) {
		        XConfigureEvent xce = ev.xconfigure;
		        
		    }
		    else if (ev.type == ClientMessage){
		    	isRunning = 0;
		    }
    	}
    	
    	RunFrame();
    	
    	/*
    	int* memWalker = (int*)bitmapData;
			
		for(int j = 0; j < height; j++){
			for(int i = 0; i < width; i++){
				*memWalker = ((unsigned char)i+xOff) | (((unsigned char)j) << 8) | 0xFF0000;
				memWalker++;
			}
		}
		*/
		
		//Flip the image upside down, cause linux
		int* memWalker = (int*)bitmapData;
		
		for(int j = 0; j < frameHeight/2; j++){
			for(int i = 0; i < frameWidth; i++){
				int fromIndex = j*frameWidth+i;
				int toIndex = (frameHeight - 1 - j)*frameWidth+i;
				
				int temp = memWalker[toIndex];
				memWalker[toIndex] = memWalker[fromIndex];
				memWalker[fromIndex] = temp;
			}
		}
		
		XPutImage(
	    display, pixmap,
	    gc, img,
	    0, 0,
	    0, 0,
	    frameWidth, frameHeight);
   		
	   	XCopyArea(display, pixmap, window, gc,
				  0, 0,
				  frameWidth, frameHeight,
				  0, 0
				  );
    }
    
    XCloseDisplay(display);
    
    return 0;
}
