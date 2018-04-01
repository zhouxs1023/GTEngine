// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in float inSize;

out VS_STRUCT
{
    vec3 position;
    vec3 color;
    float size;
} vertex;

void main()
{
    vertex.position = inPosition;
    vertex.color = inColor;
    vertex.size = inSize;
}
