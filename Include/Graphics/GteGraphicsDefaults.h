// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.25.0 (2019/04/17)

#pragma once

#include <GTEngineDEF.h>
#include <string>

#if defined(__MSWINDOWS__)
#if defined(GTE_DEV_OPENGL)
namespace gte
{
#define DefaultDepthRange false
#define DefaultEngine WGLEngine
#define DefaultProgramFactory GLSLProgramFactory

    inline std::string DefaultShaderName(std::string const& name)
    {
        return name + ".glsl";
    }
}
#else
namespace gte
{
#define DefaultDepthRange true
#define DefaultEngine DX11Engine
#define DefaultProgramFactory HLSLProgramFactory

    inline std::string DefaultShaderName(std::string const& name)
    {
        return name + ".hlsl";
    }
}
#endif
#endif

#if defined(__LINUX__)
namespace gte
{
#define DefaultDepthRange false
#define DefaultEngine GLXEngine
#define DefaultProgramFactory GLSLProgramFactory

    inline std::string DefaultShaderName(std::string const& name)
    {
        return name + ".glsl";
    }
}
#endif
