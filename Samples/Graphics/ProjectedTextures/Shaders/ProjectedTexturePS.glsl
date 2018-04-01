// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec4 projectorTCoord;

layout(location = 0) out vec4 pixelColor;

uniform sampler2D baseSampler;

void main()
{
    vec2 tcoord = projectorTCoord.xy / projectorTCoord.w;
    vec4 baseColor = texture(baseSampler, tcoord);
    pixelColor = baseColor * vertexColor;
}
