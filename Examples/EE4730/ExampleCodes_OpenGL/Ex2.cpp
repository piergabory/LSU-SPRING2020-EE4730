//Coordinate system on the window and different primitives
#include <GL/glut.h>

void displayPoints(void)
{
	glClear (GL_COLOR_BUFFER_BIT);
	glPointSize(5);
	glBegin(GL_POINTS);
		glColor3f(1, 1, 1); // white
	glVertex3f(0, 0, 0);
		glColor3f(1, 0, 0); // red	
	glVertex3f(0.5, 0, 0);
	//glColor3f(0, 1, 0); // green	
	glVertex3f(0, 0.5, 0);
	//glColor3f(0, 0, 1); // blue
	glVertex3f(0.5, 0.5, 0);	
	//glColor3f(1, 1, 0); // yellow
	glVertex3f(-0.5, 0, 0);
	//glColor3f(1, 0, 1); // purple
	glVertex3f(0, -0.5, 0);
	glEnd();		
	glFlush();
}

void displayOtherPrimitives(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1, 0, 0); // red

	//GL_POINTS
	//GL_LINES; GL_LINE_STRIP; GL_LINE_LOOP;
	//GL_TRIANGLES; GL_TRIANGLE_STRIP; GL_TRIANGLE_FAN
	//GL_QUADS; GL_QUAD_STRIP; 
	//GL_POLYGON: need to be convex and simple

	glBegin(GL_LINES);

	glVertex3f(0, 0, 0);
	glVertex3f(0.5, 0, 0);
	
	glVertex3f(0, 0.5, 0);

	glColor3f(0, 1, 0); // green


	glVertex3f(0.5, 0.5, 0);
	glVertex3f(-0.5, 0, 0);
	glVertex3f(0, -0.5, 0);
	glEnd();

	/*
	glBegin(GL_TRIANGLES);
	glColor3f(0, 0, 1);
	glVertex2f(0, 0);
	glColor3f(0, 1, 0);
	glVertex2f(1, 0);
	glColor3f(1, 0, 0);
	glVertex2f(1, 1);
	glEnd();*/
	
	glFlush();
}

int main (int argc, char **argv)
{
	glutInit (&argc,argv);
	glutCreateWindow ("The Coordinate System and Different OpenGL Primitives");
	//glutDisplayFunc(displayPoints);
	glutDisplayFunc(displayOtherPrimitives);
	glutMainLoop();
	return 0;
}
