#include <iostream>
#include <stdlib.h>

#include <GL/glut.h>

using namespace std;

char keyControl;
int mouseButton;

//Called when a key is pressed
void handleKeypress(unsigned char key, int x, int y) {
	switch (key) {
	case 'r': 
		keyControl = 'r';
		break;
	case 't':
		keyControl = 't';
		break;
	case 'z':
		keyControl = 'z';
		break;
	case 27: //Escape key
		exit(0);
	}
}

/* Some variables to measure mouse movement	*/
int mousePositionX0 = 0, mousePositionY0 = 0;

//Initializes 3D rendering
void initRendering() {
	glClearColor(0, 0, 0, 1);
	keyControl = 0;
}

//Called when the window is resized
void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
}

float obj2_angle = 0.0f;

float obj1_angle_x = 0.0f;
float obj1_angle_y = 0.0f;

float obj1_trans[2] = { 0.0f, 0.0f };
float camera_zoom = 0.0f;

//Draws the 3D scene
void drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);
	glTranslatef(0.0f, 0.0f, -5.0f);
		
	glTranslatef(0.0f, 0.0f, camera_zoom);

	//Draw Object 1
	glPushMatrix();
		glTranslatef(0.0f, -1.0f, 0.0f);
		glTranslatef(obj1_trans[0], obj1_trans[1], 0);
		glRotatef(obj1_angle_x, 1.0f, 0.0f, 0.0f);
		glRotatef(obj1_angle_y, 0.0f, 1.0f, 0.0f);
		//Trapezoid
		glBegin(GL_QUADS);
			glColor3f(0.5f, 0.0f, 0.8f);
			glVertex3f(-0.7f, -0.5f, 0.0f);

			glColor3f(0.0f, 0.9f, 0.0f);
			glVertex3f(0.7f, -0.5f, 0.0f);

			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(0.4f, 0.5f, 0.0f);

			glColor3f(0.0f, 0.65f, 0.65f);
			glVertex3f(-0.4f, 0.5f, 0.0f);	
		glEnd();
	glPopMatrix();

	//Triangle
	glPushMatrix();
		glTranslatef(-1.0f, 1.0f, 0.0f);
		glRotatef(obj2_angle, 1.0f, 2.0f, 3.0f);
	
		glBegin(GL_TRIANGLES);
			glColor3f(1.0f, 0.7f, 0.0f);
			glVertex3f(0.5f, -0.5f, 0.0f);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(0.0f, 0.5f, 0.0f);
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3f(-0.5f, -0.5f, 0.0f);	
		glEnd();
	
	glPopMatrix();
	
	glutSwapBuffers();
}

void update(int value) {
	obj2_angle += 2.0f;
	if (obj2_angle > 360) {
		obj2_angle -= 360;
	}
	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}


void mouseClick(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		mouseButton = GLUT_LEFT_BUTTON;
	else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
		mouseButton = GLUT_MIDDLE_BUTTON;
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		mouseButton = GLUT_RIGHT_BUTTON;
	mousePositionX0 = x;
	mousePositionY0 = y;
	return;
}

void mouseMove(int x, int y)
{
	float frictionFactor = 0.02f;  // just a scaling factor to make the mouse moving not too sensitive
	/* rotation*/
	if (mouseButton == GLUT_LEFT_BUTTON)
	{//Rotation
		if (keyControl == 'r'){
			int delta_x = x - mousePositionX0;
			int delta_y = y - mousePositionY0;
			obj1_angle_y += delta_x;
			obj1_angle_x += delta_y;
		}
		else if (keyControl == 't'){
			obj1_trans[0] += frictionFactor * (x - mousePositionX0);
			obj1_trans[1] += frictionFactor * (mousePositionY0 - y);
		}
		else if (keyControl == 'z'){
			camera_zoom += frictionFactor * (y - mousePositionY0);
		}
	}

	if (mouseButton == GLUT_MIDDLE_BUTTON)
	{
		////////////do something ////////////////
	}

	/* zoom in and out */
	if (mouseButton == GLUT_RIGHT_BUTTON)
	{
		////
	}
	mousePositionX0 = x;
	mousePositionY0 = y;
	glutPostRedisplay();
}


int main(int argc, char** argv) {
	//Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(400, 400);
	
	//Create the window
	glutCreateWindow("Color");
	initRendering();
	
	//Set handler functions
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(handleKeypress);
	glutReshapeFunc(handleResize);
	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove);
	
	glutTimerFunc(25, update, 0); //Add a timer
	
	glutMainLoop();
	return 0;
}

