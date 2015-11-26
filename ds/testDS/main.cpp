#include <nds.h>

#define ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

struct GameObject{
	float pos[3];
	float rot[2];
};

GameObject objects[] = {{{0,0,-1}, {0,0}}, {{-0.3f,0.5f,-1.4f}, {10,4}}};
int selectedIndex = 0;

int main() {	

	//set mode 0, enable BG0 and set it to 3D
	videoSetMode(MODE_0_3D);

	// initialize gl
	glInit();
	
	// enable antialiasing
	glEnable(GL_ANTIALIAS);
	
	// setup the rear plane
	glClearColor(0,0,0,31); // BG must be opaque for AA to work
	glClearPolyID(63); // BG must have a unique polygon ID for AA to work
	glClearDepth(0x7FFF);

	//this should work the same as the normal gl call
	glViewport(0,0,255,191);
	
	//any floating point gl call is being converted to fixed prior to being implemented
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, 256.0 / 192.0, 0.1, 40);
	
	gluLookAt(	0.0, 0.0, 1.0,		//camera possition 
				0.0, 0.0, 0.0,		//look at
				0.0, 1.0, 0.0);		//up
	
	while(1) {
		for(int i = 0; i < ARRAY_COUNT(objects); i++){
			glPushMatrix();

			//move it away from the camera
			glTranslatef32(floattof32(objects[i].pos[0]), floattof32(objects[i].pos[1]), floattof32(objects[i].pos[2]));
			
			glRotateX(objects[i].rot[0]);
			glRotateY(objects[i].rot[1]);
			
			glMatrixMode(GL_MODELVIEW);
			
			//not a real gl function and will likely change
			glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

			scanKeys();
			
			u16 heldKeys = keysHeld();
			u16 releasedKeys = keysUp();
			
			if((heldKeys & KEY_UP)){ objects[selectedIndex].rot[0] += 3;}
			if((heldKeys & KEY_DOWN)){ objects[selectedIndex].rot[0] -= 3;}
			if((heldKeys & KEY_LEFT)){ objects[selectedIndex].rot[1] += 3;}
			if((heldKeys & KEY_RIGHT)){ objects[selectedIndex].rot[1] -= 3;}
			
			if((releasedKeys & KEY_A)){selectedIndex = (selectedIndex + 1) % ARRAY_COUNT(objects);}
			

			//draw the obj
			glBegin(GL_TRIANGLE);
				
				glColor3b(255,0,0);
				glVertex3v16(inttov16(-1),inttov16(-1),0);

				glColor3b(0,255,0);
				glVertex3v16(inttov16(1), inttov16(-1), 0);

				glColor3b(0,0,255);
				glVertex3v16(inttov16(0), inttov16(1), 0);
				
			glEnd();
			
			glPopMatrix(1);
				
			
		}
		
		glFlush(0);

		swiWaitForVBlank();
	}

	return 0;
}
