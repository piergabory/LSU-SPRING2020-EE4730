#include <GL/glut.h>

void display(void){
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 0.0, 0.0);
	glutWireSphere(0.5, 50, 40);
	glutSwapBuffers(); //glFlush();
}

int main(int argc, char** argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("A red sphere in a white window");
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}
