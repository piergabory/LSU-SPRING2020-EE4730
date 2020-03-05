///
///  hw1.cpp
///  EE 4730 programming 1
///
///  Created by Pierre Gabory on 04/02/2020.
///  Copyright © 2020 Pierre Gabory. All rights reserved.
///
///  Modified to support MacOS.
///  I have reimplemented the camera controls to a simple orbit mechanic
///  I have also added a display of the bounding box of a mesh. Along with origin axis of the space.
///
///  # COMPILE
///  MacOs:
///  gpp hw1.cpp -o hw1 -framework OpenGL -framework GLUT
///
///  # USAGE:
///  ./hw1 [obj_file_1] [obj_file_2] ...
///  Obj files are consecutively loaded, the camera will be centered on the last one.
///
/// # 
///
///

// MacOs
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>

// Windows / Linux
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include <vector>
#include <algorithm>
#include <cmath>

#include "Vertex.h"
#include "Point.h"
#include "Halfedge.h"
#include "Edge.h"
#include "Mesh.h"
#include "Iterators.h"



// MARK: - CLASS HEADERS -

/// Base Object Class
/// Object tree in the scene
/// Inherited by the camera, meshes, and eventually lights..
struct Object {
    Point position;
    Point rotation;
    Point scale = Point(1, 1, 1);
    std::vector<Object *> childrens;
    
    virtual void render() const;
};

/// Box aligned to the space axes.
/// Represented by two opposites points, min and max
/// - Max is the ccrner  facing direction (+X, +Y, +Z)
/// - Min is the corner facing direction (-X, -Y, -Z)
class BoundingBox: public Object {
public:
    Point min;
    Point max;
    BoundingBox(Point min, Point max): min(min), max(max) {}
    
public:
    inline Point center();
    inline float diagonal();
    void render() const override;
};

struct Origin: public Object {
    void render() const override;
};

/// Object with a mesh
/// Provides bounding box containing all the vertices
class RenderableObject: public Object {

private:
    GLenum type = GL_TRIANGLES;
    Mesh *mesh;
    BoundingBox *bounds = nullptr;
    std::vector<Point> faceNormals;
    
    
    void computeBoundingBox();
    void computeFaceNormals();
    void computeVertexNormals();
    
public:
    Point color = Point(1, 1, 1);
    
    RenderableObject(Mesh *mesh);
    ~RenderableObject();
    
    inline void setRenderingStyle(GLenum mode);
    inline const BoundingBox& getBounds();
    inline void showBounds();
    void render() const override;
};



/// Point of view of the scene
/// Defines and updates the matrix projection
class Camera: public Object {
private:
    float fieldOfView = 45.0f;
    int width = 400;
    int height = 400;
    float nearField = 1.0;
    float farField = 1000.0;
    
    void updateView() const;
    
public:
    void resize(int newWidth, int newHeight);
    
    // Getters
    inline const int getWidth() const { return width; }
    inline const int getHeight() const { return height; }
};



/// Renderer static class
/// Provides the draw and update loop to OpenGL
/// Usage: Init is called in `main()` first.
/// second comes `createWindow()` where the app window will appear
/// finally `start()` launches the render loop.
///
/// The camera property is the point of view and root of the scene graph.
class Renderer {
private:
    static void draw();
    static void update(int value);
    static void handleResize(int w, int h);
    
public:
    static Camera camera;
    
    static void init(int &argc, char** &argv);
    static void createWindow(int width, int height, const char title[]);
    static void start();
};



/// Orbital Camera controls
/// Simple and classic orbital camera control scheme using mouse input.
/// Takes the Renderer::camera and rotates it around the (0,0,0) scene coordinates.
/// The camera up direction is always alined to the Z axis
class OrbitControls {
private:
    static void mouseMove(int screenX, int screenY);
public:
    static void init();
};


// MARK: - MAIN -

int main(int argc, char** argv) {
    
    Object *scene = new Object();
    Renderer::camera.childrens.push_back(scene);
    
    Origin origin;
    scene->childrens.push_back(&origin);
    
    RenderableObject *lastAddedObject = nullptr;
    for (int arg = 1; arg < argc; arg ++) {
        
        Mesh *mesh = new Mesh();
        // try to load obj, and push into the scene
        if (mesh->readOBJFile(argv[arg])) {
            RenderableObject *object = new RenderableObject(mesh);
            object->setRenderingStyle(GL_TRIANGLES);
            object->showBounds();
            scene->childrens.push_back(object);
            lastAddedObject = object;
        }
        
        // Free memory if failed to open file
        else {
            delete mesh;
        }
    }
    
    // center camera point of view on the last mesh
    // we move the scene so the orbit is around the object
    // then we translate the camera back so it fits in the fov.
    if (lastAddedObject != nullptr) {
        BoundingBox bounds = lastAddedObject->getBounds();
        scene->position = bounds.center() * -1;
        Renderer::camera.position.v[2] = -1.5 * bounds.diagonal();
    }
    
    Renderer::init(argc, argv);
    Renderer::createWindow(800, 800, "Prog2");
    OrbitControls::init();
    Renderer::start();
    
    return 0;
}








// MARK: - CLASS IMPLEMENTATION -



// MARK: - RenderableObject

RenderableObject::RenderableObject(Mesh *mesh): mesh(mesh), faceNormals(0) {
    computeBoundingBox();
    computeFaceNormals();
    computeVertexNormals();
}


// MARK: RenderableObject Render

void RenderableObject::render() const  {
    Object::render();
    glEnable(GL_LIGHTING);
    glBegin(type);
    
    int faceIndex = 0;

    // iterate through faces, then vertices
    for (MeshFaceIterator faceIt(mesh); !faceIt.end(); ++faceIt) {
        Point normal = faceNormals[faceIndex];
        for (FaceVertexIterator vertexIt(*faceIt); !vertexIt.end(); ++vertexIt) {
            glNormal3dv(normal.v);
            glVertex3dv((*vertexIt)->point().v);
        }
        faceIndex++;
    }
    glEnd();
}

// MARK: Compute Bounding Box

void RenderableObject::computeBoundingBox() {
    MeshVertexIterator it(mesh);
    
    Point min = (*it)->point();
    Point max = (*it)->point();
    
    for (MeshVertexIterator it(mesh); !it.end(); ++it) {
        min.v[0] = std::min(min.v[0], (*it)->point().v[0]);
        min.v[1] = std::min(min.v[1], (*it)->point().v[1]);
        min.v[2] = std::min(min.v[2], (*it)->point().v[2]);
        
        max.v[0] = std::max(max.v[0], (*it)->point().v[0]);
        max.v[1] = std::max(max.v[1], (*it)->point().v[1]);
        max.v[2] = std::max(max.v[2], (*it)->point().v[2]);
    }
    
    bounds = new BoundingBox(min, max);
}

// MARK: Mesh extras

RenderableObject::~RenderableObject() {
    for (Object *child: childrens) { delete child; }
    if (mesh != nullptr) {
        delete mesh;
    }
}

void RenderableObject::setRenderingStyle(GLenum mode) {
    type = mode;
}

const BoundingBox& RenderableObject::getBounds() {
    if (bounds == nullptr) { computeBoundingBox(); }
    return *bounds;
}


void RenderableObject::showBounds() {
    if (bounds == nullptr) { computeBoundingBox(); }
    childrens.push_back(bounds);
}


void RenderableObject::computeFaceNormals() {
    faceNormals.empty();
    
    for (MeshFaceIterator faceIt(mesh); !faceIt.end(); ++faceIt) {
        
        FaceVertexIterator vertexIt(*faceIt);
        
        // collect face vectors
        Point v0, v1, v2;
        v0 = (*vertexIt)->point();
        ++vertexIt;
        v1 = (*vertexIt)->point();
        ++vertexIt;
        v2 = (*vertexIt)->point();
        
        // compute halfedge uv vectors
        Point u = v0 - v1;
        Point v = v0 - v2;
        
        // cross product uv
        Point normal(
                     u.v[1] * v.v[2] - u.v[2] * v.v[1],
                     u.v[0] * v.v[2] - u.v[2] * v.v[0],
                     u.v[1] * v.v[0] - u.v[0] * v.v[1]);
        
        // normalize
        normal = Point(normal.v[0] / normal.norm(), normal.v[1] / normal.norm(), normal.v[2] / normal.norm());
        
        // store normal
        faceNormals.push_back(normal);
    }
}


void RenderableObject::computeVertexNormals() {
    
}


// MARK: - BoundingBox

Point BoundingBox::center() {
    return (min + max) / 2;
}

float BoundingBox::diagonal() {
    return (max - min).norm();
}

void BoundingBox::render() const {
    Point boxVertices[8] {
        min,                                    // 0
        Point(max.v[0], min.v[1], min.v[2]),    // 1
        Point(max.v[0], max.v[1], min.v[2]),    // 2
        Point(min.v[0], max.v[1], min.v[2]),    // 3
        max,                                    // 4
        Point(min.v[0], max.v[1], max.v[2]),    // 5
        Point(min.v[0], min.v[1], max.v[2]),    // 6
        Point(max.v[0], min.v[1], max.v[2])     // 7
    };
    
    int boxIndices[36] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 6, 1, 7, 2, 4, 3, 5, 0, 3, 1, 2, 4, 7, 5, 6};

    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3d(0, 1, 0);
    for (int i = 0; i < 36; i++) {
        glVertex3dv(boxVertices[boxIndices[i]].v);
        
    }
    glEnd();
}


// MARK: - Origin

void Origin::render() const {
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    
    glColor3d(1, 0, 0);
    glVertex3d(1, 0, 0);
    glVertex3d(0, 0, 0);
    
    glColor3d(0, 1, 0);
    glVertex3d(0, 1, 0);
    glVertex3d(0, 0, 0);
    
    glColor3d(0, 0, 1);
    glVertex3d(0, 0, 1);
    glVertex3d(0, 0, 0);
    
    glEnd();
}




// MARK: - Renderer

Camera Renderer::camera;

void Renderer::init(int &argc, char** &argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
}

void Renderer::createWindow(int width, int height, const char title[]) {
    glutInitWindowSize(width, height);
    glutCreateWindow(title);
    
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    float position[4] = { -10, -10, -10, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    handleResize(width, height);
}

void Renderer::start() {
    glutDisplayFunc(Renderer::draw);
    glutReshapeFunc(Renderer::handleResize);
    glutTimerFunc(25, Renderer::update, 0); //Add a timer
    glutMainLoop();
}

void Renderer::handleResize(int w, int h) {
    camera.resize(w, h);
    glutPostRedisplay();
}

void Renderer::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    camera.render();
    glPopMatrix();
    glutSwapBuffers();
}

void Renderer::update(int value) {
    glutPostRedisplay();
    glutTimerFunc(25, Renderer::update, 0);
}




// MARK: - Orbit Controls

void OrbitControls::mouseMove(int screenX, int screenY) {
    float viewX = (float)screenX / (float)Renderer::camera.getWidth();
    float viewY = (float)screenY / (float)Renderer::camera.getHeight();
    
    Point rotation = Point((viewY * 2) - 1, (viewX * 2) - 1, 0);
    Renderer::camera.rotation = rotation * 180;
}

void OrbitControls::init() {
    glutMotionFunc(OrbitControls::mouseMove);
}



// MARK: - Object

void Object::render() const {
    glTranslatef(position.v[0], position.v[1], position.v[2]);
    glRotatef(rotation.v[0], 1, 0, 0);
    glRotatef(rotation.v[1], 0, 1, 0);
    glRotatef(rotation.v[2], 0, 0, 1);
    glScaled(scale.v[0], scale.v[1], scale.v[2]);
    
    for (Object *child: childrens) {
        glPushMatrix();
        child->render();
        glPopMatrix();
    }
}




// MARK: - Camera

void Camera::updateView() const {
    float aspectRatio = (float)width / (float)height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fieldOfView, aspectRatio, nearField, farField);
}

void Camera::resize(int newWidth, int newHeight){
    width = newWidth;
    height = newHeight;
    updateView();
}
