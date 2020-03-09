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
#include <iostream>

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
    
    virtual void update() {
        for (Node *child: childrens) {
            child->update();
        }
    }
    
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
    std::vector<std::vector<Halfedge *>> boundaryEdgeLoops;
    std::vector<double> vertexGaussianCurvature;
    std::vector<short> vertexGaussianCurvatureLocalMinMax;
    
public:
    Object(Mesh *mesh): mesh(mesh), faceNormals(0) {
        computeBoundingBox();
        computeHalfEdgeAngles();
        computeFaceNormals();
        computeVertexNormals();
        computeBoundaryEdgeLoops();
        computeGaussianCurvature();
        computeGaussianCurvatureLocalMinMax();
        
        std::cout << "Found " << boundaryEdgeLoops.size() << " boundary edge loops." << std::endl;
    }
    
    ~Object()  {
        if (mesh != nullptr) {  delete mesh; }
    }
    
    inline const BoundingBox& getBounds() {
        if (bounds == nullptr) { computeBoundingBox(); }
        return *bounds;
    }
    
    // MARK: MESH RENDER
    void render() const override  {
        Node::render();
        
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glBegin(GL_TRIANGLES);
        
        
        // iterate through faces, then vertices
        for (MeshFaceIterator faceIt(mesh); !faceIt.end(); ++faceIt) {
            for (FaceVertexIterator vertexIt(*faceIt); !vertexIt.end(); ++vertexIt) {
                int index = (*vertexIt)->index();
                
                float color[4] = { 1, 1, 1, 1};
                if (showGaussianCurvatureHeatMap) {
                    switch (vertexGaussianCurvatureLocalMinMax[index]) {
                        case  1: color[0] = 1;   color[1] = 0;   color[2] = 0; break;
                        case -1: color[0] = 0;   color[1] = 1;   color[2] = 0; break;
                        case  0:
                            color[0] = 0.7 + vertexGaussianCurvature[index] * 5;
                            color[1] = 0.7 - vertexGaussianCurvature[index] * 5;
                            color[2] = 0.7;
                            break;
                    }
                }
                
                glColor3fv(color);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
                
                
                if (showGaussianCurvatureHeatMap) {
                    switch (vertexGaussianCurvatureLocalMinMax[index]) {
                        case  1:case -1: color[3] = 1; break;
                        case  0: color[3] = 0.9; break;
                    }
                }
                glMaterialfv(GL_FRONT, GL_AMBIENT, color);
                
                glNormal3dv(vertexNormals[index].v);
                glVertex3dv((*vertexIt)->point().v);
            }
        }
        glEnd();
        
        if (showBoundingBox) renderBoundingBox();
        if (showEdgeGraph) renderEdgeConnectivity();
        if (showBoundaryEdgeLoops) renderBoundaryEdgeLoops();
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
    
    // MARK: Compute angles
    void computeHalfEdgeAngles() {
        halfEdgeAngles.empty();
        halfEdgeAngles.resize(mesh->numFaces() * 3);
        
        for (MeshHalfedgeIterator heit(mesh); !heit.end(); ++heit) {
            Halfedge *he = (*heit);
            Point u = he->target()->point() - he->source()->point();
            he = he->ccw_rotate_about_source();
            if (!he) continue; //
            Point v = he->target()->point() - he->source()->point();
            
            halfEdgeAngles[he->index()] = acos((u * v) / (u.norm() * v.norm()));
        }
    }
    
    // MARK: Compute face normals
    void computeFaceNormals()  {
        faceNormals.empty();
        faceNormals.resize(mesh->numFaces());
        
        for (MeshFaceIterator faceIt(mesh); !faceIt.end(); ++faceIt) {
            Halfedge *he = (*faceIt)->he();
            Point u = he->target()->point() - he->source()->point();
            he = he->clw_rotate_about_source();
            if (!he) continue; //
            Point v = he->target()->point() - he->source()->point();
            
            Point normal = u ^ v;                      // cross product uv
            normal /= normal.norm();                   // normalize
            faceNormals[(*faceIt)->index()] = normal;  // memorize
        }
    }
    
    // MARK: Compute vertex normals
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
    
    // MARK: Compute Edge loops
    void computeBoundaryEdgeLoops() {
        boundaryEdgeLoops.empty();
        
        std::vector<bool> visited;
        visited.assign(mesh->numEdges(), false);
        
        // find a boundary vertex
        for (MeshEdgeIterator edit(mesh); !edit.end(); ++edit) {
            Edge *edge = *edit;
            if (!edge->boundary()) continue;
            if (visited[!edge->index()]) continue;
            
            std::vector<Halfedge *> loop;
            Halfedge *he = edge->he(0);
            
            // explore loop.
            int firstIndex = he->index();
            do {
                visited[he->edge()->index()] = true;
                loop.push_back(he);
                he = he->target()->most_clw_out_halfedge();
            } while (firstIndex != he->index());
            
            boundaryEdgeLoops.push_back(loop);
        }
    }
    
    
    // MARK: Compute gaussian curvature
    void computeGaussianCurvature() {
        const float tau = 2 * 3.14159265359;
        
        vertexGaussianCurvature.empty();
        vertexGaussianCurvature.assign(mesh->numVertices(), tau);
        
        for (MeshHalfedgeIterator it(mesh); !it.end(); ++it) {
            Halfedge *he = *it;
            vertexGaussianCurvature[he->source()->index()] -= halfEdgeAngles[he->index()];
        }
    }
    
    void computeGaussianCurvatureLocalMinMax() {
        // compute local min-max
        vertexGaussianCurvatureLocalMinMax.empty();
        vertexGaussianCurvatureLocalMinMax.resize(mesh->numVertices());
        
        for (MeshVertexIterator it(mesh); !it.end(); ++it) {
            int index = (*it)->index();
            const double threshold = 0.05;
            double localCurvature = vertexGaussianCurvature[index];
            
            bool discard = false;
            
            if (abs(localCurvature) < threshold) {
                discard = true;
            }
            
            else for (VertexOutHalfedgeIterator veit(*it); !veit.end(); ++veit) {
                if (localCurvature > 0 && localCurvature < vertexGaussianCurvature[(*veit)->target()->index()]) {
                    discard = true;
                };
                
                if (localCurvature < 0 && localCurvature > vertexGaussianCurvature[(*veit)->target()->index()]) {
                    discard = true;
                };
            }
            
            if (!discard) {
                vertexGaussianCurvatureLocalMinMax[index] = localCurvature > 0 ? 1 : -1;
            } else {
                vertexGaussianCurvatureLocalMinMax[index] = 0;
            }
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
        glEnable(GL_DEPTH_TEST);
        
        glBegin(GL_LINES);
        glColor3d(0, 1, 0);
        glLineWidth(1);
        
        for (int i = 0; i < 36; i++) {
            glVertex3dv(boxVertices[boxIndices[i]].v);
            
        }
        glEnd();
    }
    
    // MARK: Render edge connectivity
    void renderEdgeConnectivity() const {
        glDisable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        
        glBegin(GL_LINES);
        for (MeshEdgeIterator eit(mesh); !eit.end(); ++eit) {
            Vertex *source = (*eit)->he(0)->source();
            Vertex *target = (*eit)->he(0)->target();
            
            // Render edge at a slight offset in front of the face to avoid Z-fighting, using the normal.
            glColor3f(0, 1, 1);
            glVertex3dv((vertexNormals[source->index()] * -0.003 + source->point()).v);
            glColor3f(0, 0, 1);
            glVertex3dv((vertexNormals[target->index()] * -0.003 + target->point()).v);
        }
        glEnd();
    }
    
    
    
    // MARK: Render boundary edge loops
    void renderBoundaryEdgeLoops() const {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        
        for (std::vector<Halfedge *> loop: boundaryEdgeLoops) {
            glBegin(GL_LINES);
            glColor3d(1, 1, 0);
            glLineWidth(3.0);
            
            for (Halfedge *he: loop) {
                glVertex3dv(he->source()->point().v);
                glVertex3dv(he->target()->point().v);
            }
            glEnd();
        }
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
    
public:
    Point movement = Point(0,0,0);
    
private:
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
    
    void update() override {
        position += movement;
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
        camera.update();
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
        glEnable(GL_SMOOTH);
        glEnable(GL_LINE_SMOOTH);
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
    
    static void changeXAxis(int value) {
        Renderer::camera.movement = Point(value, 0, 0) * 0.1;
    }
    
    static void changeYAxis(int value) {
        Renderer::camera.movement = Point(0, 0, value) * 0.1;
    }
};


// MARK: - KEYBOARD HANDLER

/// Static class, listens for keyboard input, controlling display modes of Object

class KeyboardHandler final {
private:
    static void handleKeyUp(unsigned char keyCode, int screenX, int screenY) {
        switch (keyCode) {
            case 'W': case 'w': case 'Z': case 'z': case 'S': case 's':
                OrbitControls::changeYAxis(0);
                break;
                
            case 'A': case 'a': case 'Q': case 'q': case 'D': case 'd':
                OrbitControls::changeXAxis(0);
                break;
                
            case 'h': case 'H': Object::showBoundingBox ^= true;              break;
            case 'e': case 'E': Object::showEdgeGraph ^= true;                break;
            case 'b': case 'B': Object::showBoundaryEdgeLoops ^= true;        break;
            case 'k': case 'K': Object::showGaussianCurvatureHeatMap ^= true; break;
                
            default: break;
        }
    }
    
    static void handleKeyDown(unsigned char keyCode, int screenX, int screenY) {
        switch (keyCode) {
                // switch display options
            case 'W': case 'w':
            case 'Z': case 'z':
                OrbitControls::changeYAxis(1);
                break;
                
            case 'A': case 'a':
            case 'Q': case 'q':
                OrbitControls::changeXAxis(-1);
                break;
                
            case 'S': case 's':
                OrbitControls::changeYAxis(-1);
                break;
                
            case 'D': case 'd':
                OrbitControls::changeXAxis(1);
                break;
                
            default:
                break;
        }
    }
    
public:
    static void init() {
        glutKeyboardUpFunc(handleKeyUp);
        glutKeyboardFunc(handleKeyDown);
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
