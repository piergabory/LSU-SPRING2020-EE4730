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
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>







// MARK: - CLASS HEADERS -

/// Simple vector class with basic arithmetic
struct Vector3D {
    float x, y, z;
    Vector3D(float x = 0, float y = 0, float z = 0): x(x), y(y), z(z) {}
    inline float magnitudeSquared() const;
    inline float magnitude() const;
    inline float* data();
    
    // Vector arithmetic
    inline void operator +=(Vector3D v);
    inline Vector3D operator +(Vector3D v) const;
    inline Vector3D operator -(Vector3D v) const;
    inline Vector3D operator +(float s) const;
    inline Vector3D operator -(float s) const;
    inline Vector3D operator *(float s) const;
    inline Vector3D operator /(float s) const;
};



/// Box aligned to the space axes.
/// Represented by two opposites points, min and max
/// - Max is the ccrner  facing direction (+X, +Y, +Z)
/// - Min is the corner facing direction (-X, -Y, -Z)
struct BoundingBox {
    Vector3D min;
    Vector3D max;
    BoundingBox(Vector3D min, Vector3D max): min(min), max(max) {}
    
    inline Vector3D center() const;
    inline float diagonal() const;
};



/// Base Object Class
/// Object tree in the scene
/// Inherited by the camera, meshes, and eventually lights..
struct Object {
    Vector3D position;
    Vector3D rotation;
    Vector3D scale = Vector3D(1, 1, 1);
    std::vector<Object *> childrens;
    
    virtual void render() const;
};



/// Mesh Class
/// Collection of faces
/// Provides bounding box containing all the vertices
class Mesh: public Object {
typedef Vector3D Vertex;
private:
    GLenum type = GL_TRIANGLES;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> faces;
    BoundingBox *bounds = nullptr;

    void computeBoundingBox();
    
public:
    Vector3D color = Vector3D(1, 1, 1);
    
    Mesh() {}
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> faces);
    ~Mesh();
    
    inline void setRenderingStyle(GLenum mode);
    inline const BoundingBox& getBounds();
    inline void showBounds();
    void render() const override;
    bool fromOBJFile(const char filename[]);
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


/// Factory function, generates an origin axis mesh object.
Object* makeOrigin() {
    Mesh *x, *y, *z;
    x = new Mesh({ Vector3D(-1, 0, 0), Vector3D( 1, 0, 0) }, { 0, 1, 0 }); // Red X
    y = new Mesh({ Vector3D( 0,-1, 0), Vector3D( 0, 1, 0) }, { 0, 1, 0 }); // Green Y
    z = new Mesh({ Vector3D( 0, 0,-1), Vector3D( 0, 0, 1) }, { 0, 1, 0 }); // Blue Z
    
    x->color = Vector3D( 1, 0, 0);
    y->color = Vector3D( 0, 1, 0);
    z->color = Vector3D( 0, 0, 1);
    
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
    
    Object *scene = new Object();
    Renderer::camera.childrens.push_back(scene);
    
    Object *origin = makeOrigin();
    scene->childrens.push_back(origin);
    
    Mesh *lastmesh = nullptr;
    for (int arg = 1; arg < argc; arg ++) {
        
        Mesh *mesh = new Mesh();
        
        // try to load obj, and push into the scene
        if (mesh->fromOBJFile(argv[arg])) {
            mesh->setRenderingStyle(GL_TRIANGLES);
            mesh->showBounds();
            
            lastmesh = mesh;
            scene->childrens.push_back(mesh);
        }
        
        // Free memory if failed to open file
        else {
            delete mesh;
        }
    }
    
    // center camera point of view on the last mesh
    // we move the scene so the orbit is around the object
    // then we translate the camera back so it fits in the fov.
    if (lastmesh != nullptr) {
        BoundingBox bounds = lastmesh->getBounds();
        scene->position = bounds.center() * -1;
        Renderer::camera.position.z = -1.5 * bounds.diagonal();
    }
    
    Renderer::init(argc, argv);
    Renderer::createWindow(800, 800, "Prog1");
    OrbitControls::init();
    Renderer::start();
    
    return 0;
}








// MARK: - CLASS IMPLEMENTATION -



// MARK: - Mesh

// MARK: Mesh Render

void Mesh::render() const  {
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

bool Mesh::fromOBJFile(const char filename[]) {
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

// MARK: Compute Bounding Box

void Mesh::computeBoundingBox() {
    Vector3D min = vertices.front();
    Vector3D max = vertices.front();
    
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

// MARK: Mesh extras

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> faces):
vertices(vertices), faces(faces) {}

Mesh::~Mesh() {
    for (Object *child: childrens) { delete child; }
}

void Mesh::setRenderingStyle(GLenum mode) {
    type = mode;
}

const BoundingBox& Mesh::getBounds() {
    if (bounds == nullptr) { computeBoundingBox(); }
    return *bounds;
}

void Mesh::showBounds() {
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
    
    childrens.push_back(boundsMesh);
}





// MARK: - BoundingBox

Vector3D BoundingBox::center() const {
    return (min + max) / 2;
}

float BoundingBox::diagonal() const {
    return (max - min).magnitude();
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
    
    Vector3D rotation = Vector3D((viewY * 2) - 1, (viewX * 2) - 1, 0);
    Renderer::camera.rotation = rotation * 180;
}

void OrbitControls::init() {
    glutMotionFunc(OrbitControls::mouseMove);
}



// MARK: - Object

void Object::render() const {
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




// MARK: - Vector 3D

float Vector3D::magnitudeSquared() const {
    return x * x + y * y + z * z;
}

float Vector3D::magnitude() const {
    return std::sqrtf(magnitudeSquared());
}

float* Vector3D::data() {
    return &x;
}

inline Vector3D Vector3D::operator +(Vector3D v) const  {
    return Vector3D(x + v.x, y + v.y, z + v.z);
}

inline void Vector3D::operator +=(Vector3D v) {
    x += v.x; y += v.y; z += v.z;
}

inline Vector3D Vector3D::operator -(Vector3D v) const {
    return Vector3D(x - v.x, y - v.y, z - v.z);
}

Vector3D Vector3D::operator +(float s) const {
    return Vector3D(x + s, y + s, z + s);
}

Vector3D Vector3D::operator -(float s) const {
    return Vector3D(x - s, y - s, z - s);
}

Vector3D Vector3D::operator *(float s) const {
    return Vector3D(x * s, y * s, z * s);
}

Vector3D Vector3D::operator /(float s) const {
    return Vector3D(x / s, y / s, z / s);
}
