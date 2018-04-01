// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

layout(location = 0) in vec3 inVertexColor;
layout(location = 1) noperspective in vec2 inPlaneConstant;

layout(location = 0) out vec4 pixelColor;
layout(location = 1) out vec4 planeConstant;

void main()
{
    pixelColor = vec4(inVertexColor, 1.0f);
    planeConstant = vec4(inPlaneConstant, 0.0f, 0.0f);
}
