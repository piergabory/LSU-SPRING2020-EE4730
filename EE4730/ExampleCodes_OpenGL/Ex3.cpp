// Draw different shapes by pressing different characters on keyboard. See the effect of window reshape.
#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>

static char d = ' ';
static double ratio=1; // ratio=w/h=500/500=1

void display()
{ 
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0,1.0,1.0);    
    if (d=='t')
    {	glBegin(GL_POLYGON);
            glVertex3f(0.00,0.00,0.0);
            glVertex3f(0.50,0.00,0.0);
            glVertex3f(0.50,0.50,0.0);
        glEnd();   
    }
    if (d=='s') glutSolidSphere(0.25,80,60);    
    if (d=='c') glutSolidCube(0.5);
    glutSwapBuffers();
    }

void reshape(int w, int h)
{
	
	std::cout << w << " " << h;
//	if (w<h)
//		h=(int)((double)w/ratio);
//	else
//		w=(int)((double)h*ratio);
	//std::cout << " reshape to: " << w << " " << h << "\n";
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
}

void mykeyboard (unsigned char key, int x, int y){
    switch (key) {
        case 't':
            d = 't';
            glutPostRedisplay ();
            break;
        case 's':
            d = 's';
            glutPostRedisplay ();
            break;
        case 'c':
            d = 'c';
            glutPostRedisplay();
            break;
        default:
            break;
    }
}

int main(int argc, char** argv)
{
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (000, 000);
    glutCreateWindow ("t = Triangle, s = Sphere, c = Cube");
    glutDisplayFunc (display);
    glutReshapeFunc (reshape);
    glutKeyboardFunc (mykeyboard);
    glutMainLoop();
    return 0;
}
