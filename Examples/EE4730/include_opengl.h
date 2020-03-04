//
//  include_opengl.h
//  EE4730
//
//  Created by Pierre Gabory on 22/01/2020.
//  Copyright © 2020 Pierre Gabory. All rights reserved.
//

#ifndef include_opengl_h
#define include_opengl_h

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

#endif /* include_opengl_h */
