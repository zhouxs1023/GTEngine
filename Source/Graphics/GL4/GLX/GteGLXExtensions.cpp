// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.3 (2018/06/21)

#include <Graphics/GL4/GteOpenGL.h>
#include <EGL/egl.h>
#include <GL/glx.h>
#include <GL/glxext.h>

bool gUseEGLGetProcAddress = false;

void* GetOpenGLFunctionPointer(char const* name)
{
    if (gUseEGLGetProcAddress)
    {
        return (void*)(*eglGetProcAddress)(name);
    }
    else
    {
        return (void*)(*glXGetProcAddress)((GLubyte const*)name);
    }
}
