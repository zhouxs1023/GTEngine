// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/04/16)

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColorSize;
layout(location = 0) out vec4 outColorSize;

void main()
{
    gl_Position = inPosition;
    outColorSize = inColorSize;
}
