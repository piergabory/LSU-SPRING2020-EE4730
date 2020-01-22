#include <GL/glut.h>

void myInit()
{
	glClearColor (1.0, 1.0, 1.0, 0.0);
	glClear (GL_COLOR_BUFFER_BIT);
	glShadeModel(GL_FLAT);
	glColor3f(1.0, 0.0, 0.0);
}

void display (void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	// You may realize the background color changes after your window size is changed. Why?
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    gluLookAt (5,0,5, 0,0,0, 0.0,1.0, 0.0);	
    glutSolidSphere(1.0,50,50);	
	//glTranslatef (2,0,0);
	//glScalef(1.0,2.0,1.0);
    //glutSolidCube(1.0);	
	glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode (GL_PROJECTION);  
	glLoadIdentity ();	
	//gluPerspective(60, 1, 1,10);							// try this viewport setting first  
	gluPerspective(60, (float)w / (float)h, 1, 10);		// then try this viewport setting
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Changing Viewport Aspect Ratio: try to change window's aspect ratio. ");
	myInit();
    glutDisplayFunc (display);
    glutReshapeFunc (reshape);
    glutMainLoop();
    return 0;
}

