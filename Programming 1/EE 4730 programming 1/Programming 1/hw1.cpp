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




// MARK: - Vector 3D

// Simple vector class with basic arithmetic
struct Vector3D {
    float x, y, z;
    
    Vector3D(float x = 0, float y = 0, float z = 0):
    x(x), y(y), z(z) {}
    
    inline float magnitudeSquared() const {
        return x * x + y * y + z * z;
    }
    
    inline float magnitude() const {
        return std::sqrtf(magnitudeSquared());
    }
    
    inline float* data() {
        return &x;
    }
    
#ifdef DEBUG
    inline void print() const {
        std::cout << "vec(" << x << "," << y << "," << z << ")\n";
    }
#endif
    
    // Vector arithmetic
    
    inline Vector3D operator +(Vector3D v) const  {
        return Vector3D(x + v.x, y + v.y, z + v.z);
    }
    
    inline void operator +=(Vector3D v) {
        x += v.x; y += v.y; z += v.z;
    }
    
    inline Vector3D operator -(Vector3D v) const {
        return Vector3D(x - v.x, y - v.y, z - v.z);
    }
    
    inline Vector3D operator +(float s) const {
        return Vector3D(x + s, y + s, z + s);
    }
    
    inline Vector3D operator -(float s) const {
        return Vector3D(x - s, y - s, z - s);
    }
    
    inline Vector3D operator *(float s) const {
        return Vector3D(x * s, y * s, z * s);
    }
    
    inline Vector3D operator /(float s) const {
        return Vector3D(x / s, y / s, z / s);
    }
};




// MARK: - Bounding Box Class

// describes a box aligned to the space, with two points at opposite corners.
struct BoundingBox {
    Vector3D min;
    Vector3D max;
    
    BoundingBox(Vector3D min, Vector3D max): min(min), max(max) {}
    
    inline Vector3D center() const {
        return (min + max) / 2;
    }
    
    inline float diagonal() const {
        return (max - min).magnitude();
    }
};


// MARK: - Object Base Class

struct Object {
    Vector3D position;
    Vector3D rotation;
    Vector3D scale = Vector3D(1, 1, 1);
    std::vector<Object *> childrens;
    
    virtual void render() const {
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation.x, 1, 0, 0);
        glRotatef(rotation.y, 0, 1, 0);
        glRotatef(rotation.z, 0, 0, 1);
        glScaled(scale.x, scale.y, scale.z);
        
        for (Object *child: childrens) {
            glPushMatrix();
            child->render();
            glPopMatrix();
        }
    }
};




// MARK: - Mesh Class

typedef Vector3D Vertex;

class Mesh: public Object {
private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> faces;
    GLenum type = GL_TRIANGLES;
    BoundingBox *bounds = nullptr;
    
    // MARK: Bounding Box Compute
    void computeBoundingBox() {
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
        
        bounds = new BoundingBox(min, max);
    }
    
public:
    Vector3D color = Vector3D(1, 1, 1);
    
    // constructors
    Mesh() {}
    
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> faces):
    vertices(vertices), faces(faces) {}
    
    ~Mesh() {
        for (Object *child: childrens) { delete child; }
    }
    
    inline void setRenderingStyle(GLenum mode) {
        type = mode;
    }
    
    inline const BoundingBox& getBounds() {
        if (bounds == nullptr) { computeBoundingBox(); }
        return *bounds;
    }
    
    inline void showBounds() {
        if (bounds == nullptr) { computeBoundingBox(); }
        Mesh *boundsMesh = new Mesh({
            bounds->min,                                            // 0
            Vertex(bounds->max.x, bounds->min.y, bounds->min.z),    // 1
            Vertex(bounds->max.x, bounds->max.y, bounds->min.z),    // 2
            Vertex(bounds->min.x, bounds->max.y, bounds->min.z),    // 3
            bounds->max,                                            // 4
            Vertex(bounds->min.x, bounds->max.y, bounds->max.z),    // 5
            Vertex(bounds->min.x, bounds->min.y, bounds->max.z),    // 6
            Vertex(bounds->max.x, bounds->min.y, bounds->max.z)     // 7
        }, { 0, 1, 2, 3, 4, 5, 6, 7, 0, 6, 1, 7, 2, 4, 3, 5, 0, 3, 1, 2, 4, 7, 5, 6});
        
        boundsMesh->setRenderingStyle(GL_LINES);
        boundsMesh->color = Vector3D(0, 1, 0);
        
        childrens.emplace_back(boundsMesh);
    }
    
    // MARK: Mesh Render
    void render() const override {
        Object::render();
        glBegin(type);
        glColor3f(color.x, color.y, color.z);
        for (int vertexIndex: faces) {
            Vertex vertex = vertices[vertexIndex];
            glVertex3fv(vertex.data());
        }
        glEnd();
    }
    
    
    // MARK: OBJ File loader
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
        
        while ( !objFile.eof() && !objFile.fail()) {
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




// MARK: - Camera class

class Camera: public Object {
private:
    float fieldOfView = 45.0f;
    int width = 400;
    int height = 400;
    float nearField = 1.0;
    float farField = 1000.0;
    
    void updateView() const {
        float aspectRatio = (float)width / (float)height;
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(fieldOfView, aspectRatio, nearField, farField);
    }
    
public:
    inline const int getWidth() const { return width; }
    inline const int getHeight() const { return height; }
    
    void resize(int newWidth, int newHeight){
        width = newWidth;
        height = newHeight;
        updateView();
    }
};



// MARK: - Renderer Static class

class Renderer {
public:
    static Camera camera;
    
    static void init(int &argc, char** &argv) {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    }
    
    static void createWindow(int width, int height, const char title[]) {
        glutInitWindowSize(width, height);
        glutCreateWindow(title);
        glClearColor(0, 0, 0, 1);
        Renderer::camera.resize(width, height);
    }
    
    static void start() {
        glutDisplayFunc(Renderer::draw);
        glutReshapeFunc(Renderer::handleResize);
        glutTimerFunc(25, Renderer::update, 0); //Add a timer
        glutMainLoop();
    }
    
    static void handleResize(int w, int h) {
        camera.resize(w, h);
        glutPostRedisplay();
    }
    
private:
    static void draw() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();
        camera.render();
        glPopMatrix();
        glutSwapBuffers();
    }
    
    static void update(int value) {
        glutPostRedisplay();
        glutTimerFunc(25, Renderer::update, 0);
    }
};

Camera Renderer::camera;



// MARK: - Orbit Controls

class OrbitControls {
private:
    //    static void handleKeypress(unsigned char key, int x, int y) {}
    //    static void mouseClick(int button, int state, int screenX, int screenY) {
    //
    //    }
    //
    static void mouseMove(int screenX, int screenY) {
        float viewX = (float)screenX / (float)Renderer::camera.getWidth();
        float viewY = (float)screenY / (float)Renderer::camera.getHeight();
        
        Vector3D rotation = Vector3D((viewY * 2) - 1, (viewX * 2) - 1, 0);
        Renderer::camera.rotation = rotation * 180;
    }
    
public:
    static void init() {
        //        glutKeyboardFunc(OrbitControls::handleKeypress);
        //        glutMouseFunc(OrbitControls::mouseClick);
        glutMotionFunc(OrbitControls::mouseMove);
    }
};


Object* makeOrigin() {
    Mesh *x, *y, *z;
    x = new Mesh({Vector3D(-1,-0,-0), Vector3D(1,0,0)}, { 0, 1, 0});
    y = new Mesh({Vector3D(-0,-1,-0), Vector3D(0,1,0)}, { 0, 1, 0});
    z = new Mesh({Vector3D(-0,-0,-1), Vector3D(0,0,1)}, { 0, 1, 0});
    
    x->color = Vector3D(1,0,0);
    y->color = Vector3D(0,1,0);
    z->color = Vector3D(0,0,1);
    
    x->setRenderingStyle(GL_LINES);
    y->setRenderingStyle(GL_LINES);
    z->setRenderingStyle(GL_LINES);
    
    Object *origin = new Object();
    
    origin->childrens.push_back(x);
    origin->childrens.push_back(y);
    origin->childrens.push_back(z);
    
    return origin;
}



// MARK: - MAIN -

int main(int argc, char** argv) {
    // place origin in view
    Object *origin = makeOrigin();
    Renderer::camera.childrens.push_back(origin);
    
    Mesh *mesh = nullptr;
    for (int arg = 1; arg < argc; arg ++) {
        // Create mesh, and place into view
        mesh = new Mesh();
        if (!mesh->fromOBJFile(argv[arg])) { return 1; }
        mesh->setRenderingStyle(GL_TRIANGLES);
        mesh->showBounds();
        Renderer::camera.childrens.push_back(mesh);
    }
    
    if (mesh != nullptr) {
        // Place camera to capture the whole mesh (last added)
        BoundingBox bounds = mesh->getBounds();
        Renderer::camera.position = (bounds.center() + Vector3D(0, 0, bounds.diagonal() * 1.5)) * -1;
    }
    
    Renderer::init(argc, argv);
    Renderer::createWindow(800, 800, "Prog1");
    OrbitControls::init();
    Renderer::start();
    
    return 0;
}

