//
//  main.cpp
//  EE 4730 programming 1
//
//  Created by Pierre Gabory on 04/02/2020.
//  Copyright © 2020 Pierre Gabory. All rights reserved.
//

// MacOs
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

// Windows / Linux
#else
#include <GL/gl.h>
#include "include_opengl.h"
#endif

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

// Simple vector class with basic arithmetic
struct Vector3D {
    float x, y, z;
    
    Vector3D(float x, float y, float z):
    x(x), y(y), z(z) {}
    
    Vector3D(): x(0), y(0), z(0) {}
    
    inline float magnitudeSquared() const {
        return x * x + y * y + z * z;
    }
    
    inline float magnitude() const {
        return std::sqrtf(magnitudeSquared());
    }
    
    inline Vector3D operator +(Vector3D v) const  {
        return Vector3D(x + v.x, y + v.y, z + v.z);
    }
    
    inline void operator +=(Vector3D v) {
        x += v.x; y += v.y; z += v.z;
    }
    
    inline Vector3D operator -(Vector3D v) const {
        return Vector3D(x - v.x, y - v.y, z - v.z);
    }
    
    inline Vector3D operator *(float s) const {
        return Vector3D(x * s, y * s, z * s);
    }
    
    inline Vector3D operator /(float s) const {
        return Vector3D(x / s, y / s, z / s);
    }
};


struct Object {
    Vector3D position;
    Vector3D rotation;
    std::vector<Object> childrens;
    
    void render() const {
        glPushMatrix();
        
        glTranslatef(position.x, position.y, position.z);
        glRotatef(1.0, rotation.x, rotation.y, rotation.z);
        
        for (Object child: childrens) {
            child.render();
        }
        
        glPopMatrix();
    }
};


// describes a box aligned to the space, with two points at opposite corners.
class BoundingBox {
private:
    Vector3D min;
    Vector3D max;
    
public:
    BoundingBox(Vector3D min, Vector3D max): min(min), max(max) {}
    
    inline Vector3D center() const {
        return (min + max) / 2;
    }
    
    inline float diagonal() const {
        return (max - min).magnitude();
    }
};



class Mesh: public Object {
private:
    std::vector<Vector3D> vertices;
    std::vector<unsigned int> faces;
    
public:
    // constructors
    Mesh() {}
    
    Mesh(std::vector<Vector3D> vertices, std::vector<unsigned int> faces):
    vertices(vertices), faces(faces) {}
    
    BoundingBox computeBoundingBox() const {
        Vector3D min;
        Vector3D max;
        
        for (Vector3D current: vertices) {
            min.x = std::min(min.x, current.x);
            min.y = std::min(min.y, current.y);
            min.z = std::min(min.z, current.z);
            
            max.x = std::max(max.x, current.x);
            max.y = std::max(max.y, current.y);
            max.z = std::max(max.z, current.z);
        }
        
        return BoundingBox(min, max);
    }
    
    void render() const {
        Object::render();
        glBegin(GL_TRIANGLES);
        for (int vertexIndex: faces) {
            Vector3D vertex = vertices[vertexIndex];
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
        glEnd();
    }
    
    /**
     * Read Wavefront OBJ Files
     *
     * Loads a simple .obj file. Only supports vectors and faces
     * @param filename path to the source file
     * @return Mesh instance pointer
     */
    bool fromOBJFile(const char filename[]) {
        std::ifstream objFile;
        objFile.open(filename);
        
        // Check if object file open successfully
        if ( !objFile.good() ) {
            objFile.close();
            std::cerr << "failed to open file " << filename << ".\n";
            return false;
        }
        
        
        vertices.clear();
        faces.clear();
        
        float vecX, vecY, vecZ;
        unsigned int idxA, idxB, idxC;
        
        while ( !objFile.eof() ) {
            char type = objFile.get();
            
            switch (type) {
                case 'v': // Vertex position
                    objFile >> vecX >> vecY >> vecZ;
                    vertices.emplace_back(vecX, vecY, vecZ);
                    break;
                    
                case 'f': // Face indexes
                    objFile >> idxA >> idxB >> idxC;
                    faces.insert(faces.end(), { idxA - 1, idxB - 1, idxC - 1 });
                    break;
                    
                default: // Supported case
                    break;
            }
        }
        
        return true;
    }
};


struct Camera: public Object {
    float fieldOfView = 45.0f;
    int width = 400;
    int height = 400;
    float nearField = 1.0;
    float farField = 200.0;
    
    void createView(const char windowTitle[]) const {
        glutInitWindowSize(width, height);
        glutCreateWindow(windowTitle);
        glClearColor(0, 0, 0, 1);
        updateView();
    }
    
    void lookAt(Vector3D target, Vector3D up = Vector3D(0, 1, 0)) const {
        gluLookAt(position.x, position.y, position.z, target.x, target.y, target.z, up.x, up.y, up.z);
    }
    
    void updateView() const {
        float aspectRatio = (float)width / (float)height;
        
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(fieldOfView, aspectRatio, nearField, farField);
    }
    
    void render() const {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        Object::render();
    }
};

class Renderer {
public:
    static Camera camera;
    
    static void init(int &argc, char** &argv) {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        Renderer::camera.position = Vector3D(0, 0, -5);
        Renderer::camera.createView("Prog 1");
    }
    
    static void start() {
        glutDisplayFunc(Renderer::draw);
        glutReshapeFunc(Renderer::handleResize);
        glutTimerFunc(25, Renderer::update, 0); //Add a timer
        glutMainLoop();
    }
    
    static void handleResize(int w, int h) {
        Renderer::camera.width = w;
        Renderer::camera.height = h;
        Renderer::camera.updateView();
        glutPostRedisplay();
    }
    
private:
    static void draw() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camera.render();
        glutSwapBuffers();
    }
    
    static void update(int value) {
        glutPostRedisplay();
        glutTimerFunc(25, Renderer::update, 0);
    }
};

Camera Renderer::camera;


class OrbitControls {
private:
//    static void handleKeypress(unsigned char key, int x, int y) {}
//    static void mouseClick(int button, int state, int x, int y) {}
    
    static void mouseMove(int x, int y) {
        Renderer::camera.rotation.x = y / 100.0;
        Renderer::camera.rotation.y = x / 100.0;
    }
        
public:
    static void init() {
//        glutKeyboardFunc(OrbitControls::handleKeypress);
//        glutMouseFunc(OrbitControls::mouseClick);
        glutMotionFunc(OrbitControls::mouseMove);
    }
};



int main(int argc, char** argv) {
    Mesh mesh;
    if (!mesh.fromOBJFile("David.obj")) { return 1; }
    BoundingBox bounds = mesh.computeBoundingBox();
    mesh.position = bounds.center() + Vector3D(0, 0, bounds.diagonal() * 1.5);
    
    Renderer::camera.childrens.push_back(mesh);
    Renderer::init(argc, argv);
    OrbitControls::init();
    
    Renderer::start();

    return 0;
}

