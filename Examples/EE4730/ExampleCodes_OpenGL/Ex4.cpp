/* Modeling and Viewing Transformations   */
#include <GL/glut.h>
   
void display (void)
{ 
    glClearColor (1.0, 1.0, 1.0, 0.0);
    glClear (GL_COLOR_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();
	gluLookAt(0, 1, 5, 0, 0, 0, 0, 1, 0);
	
	//glRotatef(45, 1, 0, 0);

	//glTranslatef(0, 0, -1);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 0.2, 6); //play with FOV and think why the results look different
	
	glMatrixMode(GL_MODELVIEW);
	
	
	glColor3f (0.0, 0.0, 0.0);	//black sphere at origin
	glutSolidSphere(0.1, 20,20);

	
	glTranslatef(0.5, 0.0, 0.0);
	glColor3f (1.0, 0.0, 0.0);	// red sphere at x+ direction
	glScalef(2.0,1.0,1.0);
	glutWireSphere(0.1,20,20);
	glScalef(0.5,1.0,1.0);
	glTranslatef(-0.5, 0.0, 0.0);

	glPushMatrix();
	glTranslatef(0.0, 0.5, 0.0);
	glColor3f (0.0, 1.0, 0.0); // green sphere at y+ direction
	glScalef(1.0,2.0,1.0);
	glutWireSphere(0.1,20,20);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 0.0, 0.5);
	glColor3f (0.0, 0.0, 1.0); // blue sphere at z+ direction
	glScalef(1.0,1.0,2.0);
	glutWireSphere(0.1,20,20);
	glPopMatrix();
	
    glutSwapBuffers();
}

int main(int argc, char** argv)
{
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Transformation");
    glutDisplayFunc (display);
    glutMainLoop();
    return 0;
}
