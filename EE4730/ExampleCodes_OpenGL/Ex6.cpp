#include <iostream>
#include <GL/glut.h>


static float eyept[3] = {2,2,2};
static float refpt[3] = {0,0,0};
static float uppt[3] = {0,1,0};
static float whratio;


void myInit()
{	
	glShadeModel (GL_FLAT);
	glClear (GL_COLOR_BUFFER_BIT);	

	glMatrixMode (GL_PROJECTION);     
	glLoadIdentity();
	gluPerspective(90, 1, 1, 10);
	glMatrixMode (GL_MODELVIEW);		//change back to modelview 	
}

void display (void)
{
	std::cout << "display is called.\n";
	glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    gluLookAt(eyept[0], eyept[1], eyept[2], refpt[0], refpt[1], refpt[2], uppt[0], uppt[1], uppt[2]);
	
	glColor3f (1,1,1);
    glutSolidSphere(0.1, 10, 10);

	glTranslatef(1,0,0);
	glColor3f (1,0,0);	
	glRotatef(90, 0, 1, 0);
	glutWireCone(0.1, 0.3, 10, 10);
	glRotatef(-90, 0, 1, 0); //why do we need this?  please consider
	glTranslatef(-1,0,0);    

	
	glColor3f (0,1,0);	
	glPushMatrix();			//using PushMatrix to keep a copy of the current transformation matrix
	glTranslatef(0,1,0);
	glRotatef(-90, 1, 0, 0);
	glutWireCone(0.1,0.3,10,10);  //restore the saved transformation matrix
	glPopMatrix();
	

	glColor3f (0,0,1);
	glPushMatrix();
	glTranslatef(0,0,1);	
	glRotatef(-90, 0, 0, 1);
	glutWireCone(0.1,0.3,10,10);
	glPopMatrix();	
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	
	glMatrixMode (GL_PROJECTION);     
	glLoadIdentity();
	whratio = (double)w/(double)h; //in general, this ratio in gluPerspective should match the aspect ratio of the associated viewport
	gluPerspective(90, whratio, 1, 10);
	glMatrixMode (GL_MODELVIEW);		//change back to modelview 	
	std::cout << "Reshape callback function is called once \n";	
	//glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize (700, 700);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Transformation");
	myInit();
    glutDisplayFunc (display);
    glutReshapeFunc (reshape);			//try to comment out this and change the window's dimensions
    glutMainLoop();
    return 0;
}

