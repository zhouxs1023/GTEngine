// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)


uniform sampler1DArray mySampler1;  // 2 textures in the array
uniform sampler2DArray mySampler2;  // 2 textures in the array

layout(location = 0) in vec2 vertexTCoord;

layout(location = 0) out vec4 pixelColor0;

void main()
{
    pixelColor0 = vec4(0, 0, 0, 0);

    vec4 tcd;

    // Sample the 1D texture array.
    tcd.xy = vec2(vertexTCoord.x, 0);
    pixelColor0 += texture(mySampler1, tcd.xy);
    tcd.xy = vec2(vertexTCoord.x, 1);
    pixelColor0 += texture(mySampler1, tcd.xy);

    // Sample the 2D texture array.
    tcd.xyz = vec3(vertexTCoord, 0);
    pixelColor0 += texture(mySampler2, tcd.xyz);
    tcd.xyz = vec3(vertexTCoord, 1);
    pixelColor0 += texture(mySampler2, tcd.xyz);

    pixelColor0 *= 0.25f;
};
