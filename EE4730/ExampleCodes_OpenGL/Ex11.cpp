#include <iostream>
#include <GL/glut.h>

float lightAngle = 0.0f;

void init(void)
{
	std::cout << "Executing the init() function. \n";
	//Define Material Properties for the Objects in the Scene
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	
	//The following color components specify the intensity for each type of lights.
	GLfloat light_ambient[] = { 0.7, 0.7, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);


	//Material properties determine how it reflects light
	//You can specify a material's ambient, diffuse, and specular colors and how shiny it is.
	//Here only the last two material properties are explicitly specified
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
		
	
	//This enables lighting calculations
	glEnable(GL_LIGHTING);
	
	//Remember to enable the light you just defined 
	glEnable(GL_LIGHT0);
	
	glEnable(GL_DEPTH_TEST);
}

void update(int value) {
	lightAngle += 2.0f;
	if (lightAngle > 360) {
		lightAngle -= 360;
	}
	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}

void display(void)
{	
	GLfloat light_position[] = { 0.0, 0.0, 1.5, 1.0 };
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
	glPushMatrix();
		glRotated(lightAngle, 1.0, 0.0, 0.0);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glPopMatrix();
	
	glutSolidIcosahedron();
	glPopMatrix();
	glFlush();
}

void reshape(int w, int h)
{
	std::cout << "Executing the reshape() function. \n";
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho(-1.5, 1.5, -1.5*(GLfloat)h / (GLfloat)w,
		1.5*(GLfloat)h / (GLfloat)w, -10.0, 10.0);
	else
		glOrtho(-1.5*(GLfloat)w / (GLfloat)h,
		1.5*(GLfloat)w / (GLfloat)h, -1.5, 1.5, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutTimerFunc(25, update, 0); //Add a timer
	glutMainLoop();
	return 0;
}