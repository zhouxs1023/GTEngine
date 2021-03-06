// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.7 (2019/04/13)

#pragma once

#include <GTEngineDEF.h>

#if defined(__MSWINDOWS__)
#if defined(GTE_DEV_OPENGL)
#include <GTGraphicsGL4.h>
#else
#include <GTGraphicsDX11.h>
#endif
#endif

#if defined(__LINUX__)
#include <GTGraphicsGL4.h>
#endif

#include <Graphics/GteGraphicsDefaults.h>
