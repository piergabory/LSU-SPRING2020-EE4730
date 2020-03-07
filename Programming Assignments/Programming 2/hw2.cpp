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



// MARK: - NODE

/// Base Node Class
/// Node tree in the scene
/// Inherited by the camera, meshes, and eventually lights..
struct Node {
    Point position;
    Point rotation;
    Point scale = Point(1, 1, 1);
    std::vector<Node *> childrens;
    
    virtual void render() const {
        glTranslatef(position.v[0], position.v[1], position.v[2]);
        glRotatef(rotation.v[0], 1, 0, 0);
        glRotatef(rotation.v[1], 0, 1, 0);
        glRotatef(rotation.v[2], 0, 0, 1);
        glScaled(scale.v[0], scale.v[1], scale.v[2]);
        
        for (Node *child: childrens) {
            glPushMatrix();
            child->render();
            glPopMatrix();
        }
    }
};



// MARK: - BOUNDING BOX

/// Box aligned to the space axes.
/// Represented by two opposites points, min and max
/// - Max is the ccrner  facing direction (+X, +Y, +Z)
/// - Min is the corner facing direction (-X, -Y, -Z)

class BoundingBox {
public:
    Point min;
    Point max;
    BoundingBox(Point min, Point max): min(min), max(max) {}
    
public:
    inline Point center() { return (min + max) / 2; }
    inline float diagonal() { return (max - min).norm(); }
};



// MARK: - ORIGIN AXIS
struct Origin final : public Node {
    void render() const override {
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
};



// MARK: - OBJECT

/// Node with a mesh
/// Provides bounding box containing all the vertices
class Object: public Node {
    
public:
    static bool showBoundingBox;
    static bool showEdgeGraph;
    static bool showBoundaryEdgeLoops;
    static bool showGaussianCurvatureHeatMap;
    
private:
    Mesh *mesh;
    BoundingBox *bounds = nullptr;
    
    std::vector<double> halfEdgeAngles;
    std::vector<Point> faceNormals;
    std::vector<Point> vertexNormals;
    
 public:
    Object(Mesh *mesh): mesh(mesh), faceNormals(0) {
        computeBoundingBox();
        computeHalfEdgeAngles();
        computeFaceNormals();
        computeVertexNormals();
    }
    
    ~Object()  {
        if (mesh != nullptr) {  delete mesh; }
    }
    
    inline const BoundingBox& getBounds() {
        if (bounds == nullptr) { computeBoundingBox(); }
        return *bounds;
    }
    
    void render() const override  {
        Node::render();
        glEnable(GL_LIGHTING);
        glBegin(GL_TRIANGLES);
        
        // iterate through faces, then vertices
        for (MeshFaceIterator faceIt(mesh); !faceIt.end(); ++faceIt) {
            for (FaceVertexIterator vertexIt(*faceIt); !vertexIt.end(); ++vertexIt) {
                glNormal3dv(vertexNormals[(*vertexIt)->index()].v);
                glVertex3dv((*vertexIt)->point().v);
            }
        }
        glEnd();
        
        if (showBoundingBox) renderBoundingBox();
    }
    
    
private:
    void computeBoundingBox() {
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
    
    void computeHalfEdgeAngles() {
        halfEdgeAngles.empty();
        halfEdgeAngles.resize(mesh->numFaces() * 3);
        
        for (MeshHalfedgeIterator heit(mesh); !heit.end(); ++heit) {
            Halfedge *he = (*heit);
            Point u = he->target()->point() - he->source()->point();
            he = he->ccw_rotate_about_source();
            Point v = he->target()->point() - he->source()->point();
            
            halfEdgeAngles[he->index()] = acos((u * v) / (u.norm() * v.norm()));
        }
    }
    
    void computeFaceNormals()  {
        faceNormals.empty();
        faceNormals.resize(mesh->numFaces());
        
        for (MeshFaceIterator faceIt(mesh); !faceIt.end(); ++faceIt) {
            // collect face points
            FaceVertexIterator vertexIt(*faceIt);
            Point v0, v1, v2;
            v0 = (*vertexIt)->point();
            ++vertexIt;
            v1 = (*vertexIt)->point();
            ++vertexIt;
            v2 = (*vertexIt)->point();
            
            // compute halfedge uv vectors
            Point u = v1 - v0;
            Point v = v2 - v0;
            
            Point normal = v ^ u;                      // cross product uv
            normal /= normal.norm();                   // normalize
            faceNormals[(*faceIt)->index()] = normal;  // memorize
        }
    }
    
    void computeVertexNormals() {
        vertexNormals.clear();
        vertexNormals.resize(mesh->numVertices());
        
        for (MeshVertexIterator veit(mesh); !veit.end(); ++veit) {
            Point normal(0,0,0);
            
            for (VertexOutHalfedgeIterator heoit(*veit); !heoit.end(); ++heoit) {
                Point weightedNormal = faceNormals[(*heoit)->face()->index()] * halfEdgeAngles[(*heoit)->index()];
                normal += weightedNormal;
            }
            
            normal /= normal.norm();                  // normalise
            vertexNormals[(*veit)->index()] = normal; // memorise
        }
    }
    
    void renderBoundingBox() const {
        Point min = bounds->min;
        Point max = bounds->max;
        
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
};

bool Object::showBoundingBox = false;
bool Object::showEdgeGraph = false;
bool Object::showBoundaryEdgeLoops = false;
bool Object::showGaussianCurvatureHeatMap = false;


// MARK: - CAMERA

/// Point of view of the scene
/// Defines and updates the matrix projection
class Camera final : public Node {
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
    void resize(int newWidth, int newHeight){
        width = newWidth;
        height = newHeight;
        updateView();
    }
    
    // Getters
    inline const int getWidth() const { return width; }
    inline const int getHeight() const { return height; }
};


// MARK: - RENDERER

/// Renderer static class
/// Provides the draw and update loop to OpenGL
/// Usage: Init is called in `main()` first.
/// second comes `createWindow()` where the app window will appear
/// finally `start()` launches the render loop.
///
/// The camera property is the point of view and root of the scene graph.
class Renderer final {
public:
    static Camera camera;
    
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
    
    static void handleResize(int w, int h) {
        camera.resize(w, h);
        glutPostRedisplay();
    }
    
public:
    static void init(int &argc, char** &argv) {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    }
    
    static void createWindow(int width, int height, const char title[]) {
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
    
    static void start() {
        glutDisplayFunc(draw);
        glutReshapeFunc(handleResize);
        glutTimerFunc(25, update, 0); //Add a timer
        glutMainLoop();
    }
};

// static camera declaration
Camera Renderer::camera;


// MARK: - ORBITAL CAMERA

/// Orbital Camera controls
/// Simple and classic orbital camera control scheme using mouse input.
/// Takes the Renderer::camera and rotates it around the (0,0,0) scene coordinates.
/// The camera up direction is always alined to the Z axis
class OrbitControls final {
private:
    static void mouseMove(int screenX, int screenY) {
        float viewX = (float)screenX / (float)Renderer::camera.getWidth();
        float viewY = (float)screenY / (float)Renderer::camera.getHeight();
        
        Point rotation = Point((viewY * 2) - 1, (viewX * 2) - 1, 0);
        Renderer::camera.rotation = rotation * 180;
    }
public:
    static void init() {
        glutMotionFunc(mouseMove);
    }
};


// MARK: - KEYBOARD HANDLER

/// Static class, listens for keyboard input, controlling display modes of Object

class KeyboardHandler final {
private:
    static void handleKeyUp(unsigned char keyCode, int screenX, int screenY) {
        switch (keyCode) {
            // switch display options
            case 's': Object::showBoundingBox ^= true;              break;
            case 'e': Object::showEdgeGraph ^= true;                break;
            case 'b': Object::showBoundaryEdgeLoops ^= true;        break;
            case 'k': Object::showGaussianCurvatureHeatMap ^= true; break;
                
            default: break;
        }
    }
    
public:
    static void init() {
        glutKeyboardUpFunc(handleKeyUp);
    }
};


// MARK: - MAIN -

int main(int argc, char** argv) {
    
    Node *scene = new Node();
    Renderer::camera.childrens.push_back(scene);
    
    Origin origin;
    scene->childrens.push_back(&origin);
    
    Object *lastAddedObject = nullptr;
    for (int arg = 1; arg < argc; arg ++) {
        
        Mesh *mesh = new Mesh();
        // try to load obj, and push into the scene
        if (mesh->readOBJFile(argv[arg])) {
            Object *object = new Object(mesh);
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
    KeyboardHandler::init();
    Renderer::start();
    
    return 0;
}
