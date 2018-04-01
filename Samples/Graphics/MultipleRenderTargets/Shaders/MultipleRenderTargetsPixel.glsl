// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

uniform FarNearRatio
{
    float farNearRatio;
};

uniform sampler2D baseSampler;

layout(location = 0) in vec2 vertexTCoord;
layout(location = 1) in float perspectiveDepth;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 screenPosition;

void main()
{
    color = texture(baseSampler, vertexTCoord);
    screenPosition = gl_FragCoord;

    // For OpenGL, d = (f+n)/(f-n) - 2*f*n/((f-n)*z) and is in [-1,+1].
    // It is the value of perspectiveDepth that gets passed in here after
    // going through perspective interpolation by the rasterizer.
    // Solve for linear depth L = (z-n)/(f-n) in [-1,+1]
    // to obtain L = (2-r*(1-d))/(r*(1-d)+2*d).

    float d = perspectiveDepth;
    gl_FragDepth = (2.0f - farNearRatio*(1.0f-d)) / (farNearRatio*(1.0f-d) + 2.0f*d);
};
