// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

uniform PVWMatrix
{
    mat4 pvwMatrix;
};

uniform ConstantColor
{
    vec4 constantColor;
};

buffer position { vec4 data[]; } positionSB;

out vec4 vertexColor;

void main()
{
    vec4 modelPosition = positionSB.data[gl_VertexID];
#if GTE_USE_MAT_VEC
    gl_Position = pvwMatrix * modelPosition;
#else
    gl_Position = modelPosition * pvwMatrix;
#endif
    vertexColor = constantColor;
}
