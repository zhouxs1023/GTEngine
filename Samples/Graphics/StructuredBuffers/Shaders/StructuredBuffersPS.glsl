// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

uniform sampler2D baseSampler;

buffer drawnPixels
{
    vec4 data[];
} drawnPixelsSB;

layout(location = 0) in vec2 vertexTCoord;

layout(location = 0) out vec4 pixelColor0;

void main()
{
    vec4 outputColor = texture(baseSampler, vertexTCoord);
    pixelColor0 = outputColor;
    ivec2 location = ivec2(gl_FragCoord.xy);
    drawnPixelsSB.data[location.x + WINDOW_WIDTH*location.y] =  outputColor;
};
