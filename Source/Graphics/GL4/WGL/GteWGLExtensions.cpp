// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2018/03/11)

#include <Graphics/GL4/GteOpenGL.h>
#include <Graphics/GL4/GL/wglext.h>
#include <cassert>

void* GetOpenGLFunctionPointer(char const* name)
{
    return (void*)wglGetProcAddress(name);
}

template <typename PWGLFunction>
static void GetWGLFunction(char const* name, PWGLFunction& function)
{
    function = (PWGLFunction)wglGetProcAddress(name);
}


static PFNWGLSWAPINTERVALEXTPROC swglSwapIntervalEXT = nullptr;
static PFNWGLGETSWAPINTERVALEXTPROC swglGetSwapIntervalEXT = nullptr;

BOOL __stdcall wglSwapIntervalEXT(int interval)
{
    BOOL result;
    if (swglSwapIntervalEXT)
    {
        result = swglSwapIntervalEXT(interval);
    }
    else
    {
        assert(false);
        result = FALSE;
    }
    return result;
}

int __stdcall wglGetSwapIntervalEXT(void)
{
    int result;
    if (swglGetSwapIntervalEXT)
    {
        result = swglGetSwapIntervalEXT();
    }
    else
    {
        assert(false);
        result = 0;
    }
    return result;
}

void InitializeWGL()
{
    GetWGLFunction("wglSwapIntervalEXT", swglSwapIntervalEXT);
    GetWGLFunction("wglGetSwapIntervalEXT", swglGetSwapIntervalEXT);
}

