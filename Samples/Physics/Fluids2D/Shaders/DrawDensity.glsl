// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

uniform sampler2D stateSampler;

layout(location = 0) in vec2 vertexTCoord;
layout(location = 0) out vec4 pixelColor;

void main()
{
    // Map velocity channels to colors and modulate by density.
    vec4 current = texture(stateSampler, vertexTCoord);
    vec3 color = 0.5f + 0.5f*current.xyz/(1.0f + abs(current.xyz));
    pixelColor = vec4(current.w*color, 1.0f);
}
