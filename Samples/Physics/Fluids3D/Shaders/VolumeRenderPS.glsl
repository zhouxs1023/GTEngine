// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

uniform sampler3D volumeSampler;

layout(location = 0) in vec3 vertexTCoord;
layout(location = 0) out vec4 pixelColor;

void main()
{
    vec4 color = texture(volumeSampler, vertexTCoord);
    pixelColor = vec4(color.w*color.rgb, 0.5f*color.w);
}
