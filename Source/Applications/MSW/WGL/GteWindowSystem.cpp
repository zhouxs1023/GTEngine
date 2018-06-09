// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2018/03/11)

#include <GTEnginePCH.h>
#include <Applications/MSW/GteMSWWindowSystem.h>
#include <Applications/MSW/WGL/GteWindow.h>
#include <Graphics/GL4/WGL/GteWGLEngine.h>
#include <Graphics/GL4/GteGLSLProgramFactory.h>
using namespace gte;

void WindowSystem::CreateEngineAndProgramFactory(MSWWindow::Parameters& parameters)
{
    Window::Parameters& p = static_cast<Window::Parameters&>(parameters);

    bool saveDriverInfo = ((p.deviceCreationFlags & 0x00000001) != 0);
    auto engine = std::make_shared<WGLEngine>(p.handle, p.useDepth24Stencil8, saveDriverInfo);
    if (!engine->MeetsRequirements())
    {
        LogError("OpenGL 4.3 or later is required.");
        parameters.engine = nullptr;
        parameters.factory = nullptr;
        parameters.created = false;
        return;
    }

    if (engine->GetDevice())
    {
        parameters.engine = engine;
        parameters.factory = std::make_shared<GLSLProgramFactory>();
        parameters.created = true;
    }
    else
    {
        LogError("Cannot create graphics engine.");
        parameters.engine = nullptr;
        parameters.factory = nullptr;
        parameters.created = false;
    }
}
