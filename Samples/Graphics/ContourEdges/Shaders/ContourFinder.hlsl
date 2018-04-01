// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.6.0 (2016/12/09)

// vertex shader
struct VS_INPUT
{
    uint vertexID : SV_VERTEXID;
};

struct VS_OUTPUT
{
    uint vertexID : BLENDINDICES0;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
    output.vertexID = input.vertexID;
    return output;
}

// geometry shader
cbuffer MeshID
{
    uint meshID;
};

cbuffer WindowExtent
{
    float2 ExEy;  // (windowWidth/2, windowHeight/2)
};

cbuffer LineThickness
{
    float R;
};

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

struct Vertex
{
    float3 modelPosition;
    float2 modelTCoord;
};

StructuredBuffer<Vertex> vertices;

struct GS_OUTPUT
{
    uint2 attributes : BLENDINDICES0;
    float depth : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

void ComputeClipPointsForCorners(in float4 HV0, in float4 HV1, in float4 HV2,
    out float4 HC00, out float4 HC01, out float4 HC10, out float4 HC11)
{
    float2 pix0 = ExEy * HV0.xy / HV0.w;
    float2 pix1 = ExEy * HV1.xy / HV1.w;
    float2 pix2 = ExEy * HV2.xy / HV2.w;
    float2 pix10 = pix1 - pix0;
    float2 pix20 = pix2 - pix0;
    float2 pix21 = pix2 - pix1;
    float2 DPerp = normalize(float2(-pix10.y, pix10.x));
    float LSqr = dot(pix10, pix10);
    float L = sqrt(LSqr);
    float LR = L * R;
    float LSqrPlusLR = LSqr + LR;
    float TwoLSqrPlusLR = LSqrPlusLR + LSqr;
    float temp = dot(pix20, pix20) - dot(pix21, pix21);
    float w0 = HV0[3], w1 = HV1[3], w2 = HV2[3];
    float invW0 = 1 / HV0[3], invW1 = 1 / HV1[3];

    float c0, c1, c2, invC1mC0, d0, d1, d2, e0, e1, t;
    float alpha0, alpha1, beta0, beta1, gamma0, gamma1;
    float q0, q1, q2, invQ2, discr, rootDiscr;
    float b0, b1, b2;
    float4 HK0, HK1;
    float2 K0, K1;

    // Generate HC00 and HC01.
    c0 = w0 * LR;
    c1 = HV1[3] * TwoLSqrPlusLR;
    c2 = HV2[3] * (LSqrPlusLR + temp);
    invC1mC0 = 1 / (c1 - c0);
    d0 = w0 * (c2 - c1) * invC1mC0;
    e0 = w0 * c1 * invC1mC0;
    d1 = w1 * (c0 - c2) * invC1mC0;
    e1 = w1 * (-c0) * invC1mC0;
    d2 = w2;
    alpha0 = R * (e0 + e1);
    alpha1 = R * (d0 + d1 + d2);
    beta0 = pix10[0] * e1;
    beta1 = pix10[0] * d1 + pix20[0] * d2;
    gamma0 = pix10[1] * e1;
    gamma1 = pix10[1] * d1 + pix20[1] * d2;
    q0 = 0.5 * alpha0 * alpha0 - beta0 * beta0 - gamma0 * gamma0;
    q1 = 0.5 * alpha0 * alpha1 - beta0 * beta1 - gamma0 * gamma1;
    q2 = 0.5 * alpha1 * alpha1 - beta1 * beta1 - gamma1 * gamma1;
    discr = q1 * q1 - q0 * q2;
    rootDiscr = sqrt(max(discr, 0));
    invQ2 = 1 / q2;
    t = (-q1 - rootDiscr) * invQ2;
    b0 = (d0 * t + e0) * invW0;
    b1 = (d1 * t + e1) * invW1;
    b2 = t;
    HK0 = b0 * HV0 + b1 * HV1 + b2 * HV2;
    if (HK0[3] < 0)
    {
        HK0 = -HK0;
    }
    K0 = ExEy * HK0.xy / HK0.w;
    t = (-q1 + rootDiscr) * invQ2;
    b0 = (d0 * t + e0) * invW0;
    b1 = (d1 * t + e1) * invW1;
    b2 = t;
    HK1 = b0 * HV0 + b1 * HV1 + b2 * HV2;
    if (HK1[3] < 0)
    {
        HK1 = -HK1;
    }
    K1 = ExEy * HV1.xy / HV1.w;
    if (dot(DPerp, K1 - K0) > 0)
    {
        HC00 = HK0;
        HC01 = HK1;
    }
    else
    {
        HC00 = HK1;
        HC01 = HK0;
    }

    // Generate HC10 and HC11.
    c0 = w0 * TwoLSqrPlusLR;
    c1 = HV1[3] * LR;
    c2 = HV2[3] * (LSqrPlusLR - temp);
    invC1mC0 = 1 / (c1 - c0);
    d0 = w0 * (c2 - c1)* invC1mC0;
    e0 = w0 * c1 * invC1mC0;
    d1 = w1 * (c0 - c2) * invC1mC0;
    e1 = w1 * (-c0) * invC1mC0;
    d2 = w2;
    alpha0 = R * (e0 + e1);
    alpha1 = R * (d0 + d1 + d2);
    beta0 = -pix10[0] * e0;
    beta1 = -pix10[0] * d0 + pix21[0] * d2;
    gamma0 = -pix10[1] * e0;
    gamma1 = -pix10[1] * d0 + pix21[1] * d2;
    q0 = 0.5 * alpha0 * alpha0 - beta0 * beta0 - gamma0 * gamma0;
    q1 = 0.5 * alpha0 * alpha1 - beta0 * beta1 - gamma0 * gamma1;
    q2 = 0.5 * alpha1 * alpha1 - beta1 * beta1 - gamma1 * gamma1;
    discr = q1 * q1 - q0 * q2;
    rootDiscr = sqrt(max(discr, 0));
    invQ2 = 1 / q2;
    t = (-q1 - rootDiscr) * invQ2;
    b0 = (d0 * t + e0) * invW0;
    b1 = (d1 * t + e1) * invW1;
    b2 = t;
    HK0 = b0 * HV0 + b1 * HV1 + b2 * HV2;
    if (HK0[3] < 0)
    {
        HK0 = -HK0;
    }
    K0 = ExEy * HK0.xy / HK0.w;
    t = (-q1 + rootDiscr) * invQ2;
    b0 = (d0 * t + e0) * invW0;
    b1 = (d1 * t + e1) * invW1;
    b2 = t;
    HK1 = b0 * HV0 + b1 * HV1 + b2 * HV2;
    if (HK1[3] < 0)
    {
        HK1 = -HK1;
    }
    K1 = ExEy * HK0.xy / HK0.w;
    if (dot(DPerp, K1 - K0) > 0)
    {
        HC10 = HK0;
        HC11 = HK1;
    }
    else
    {
        HC10 = HK1;
        HC11 = HK0;
    }

    // Use the edge endpoint depths for the corner depths to avoid z-buffer
    // aliasing in ContourDrawer.hlsl.  The aliasing is due to inaccurate
    // w-values for the corners dues to floating-point rounding errors.
    float depth = HV0[2] * invW0;
    HC00.z = HC00.w * depth;
    HC01.z = HC01.w * depth;
    depth = HV1[2] * invW1;
    HC10.z = HC10.w * depth;
    HC11.z = HC11.w * depth;
}

bool Clipper(in float test0, in float test1, inout float4 outHV0, inout float4 outHV1)
{
    if (test0 <= 0 && test1 <= 0)
    {
        return false;
    }

    if (test0 <= 0)  // test1 > 0 is necessary
    {
        float t = test0 / (test0 - test1);
        outHV0 = (1 - t) * outHV0 + t * outHV1;
    }

    if (test1 <= 0)  // test0 > 0 is necessary
    {
        float t = test0 / (test0 - test1);
        outHV1 = (1 - t) * outHV0 + t * outHV1;
    }

    return true;
}

bool ClipLine(in float4 inHV0, in float4 inHV1, out float4 outHV0, out float4 outHV1)
{
    outHV0 = inHV0;
    outHV1 = inHV1;

    // Clip so that z >= 0, ensuring w > 0.
    if (!Clipper(outHV0[2], outHV1[2], outHV0, outHV1))
    {
        return false;
    }

    // Clip so that x/w <= 1.
    if (!Clipper(outHV0[3] - outHV0[0], outHV1[3] - outHV1[0], outHV0, outHV1))
    {
        return false;
    }

    // Clip so that y/w <= 1.
    if (!Clipper(outHV0[3] - outHV0[1], outHV1[3] - outHV1[1], outHV0, outHV1))
    {
        return false;
    }

    // Clip so that z/w <= 1.
    if (!Clipper(outHV0[3] - outHV0[2], outHV1[3] - outHV1[2], outHV0, outHV1))
    {
        return false;
    }

    // Clip so that x/w >= -1.
    if (!Clipper(outHV0[3] + outHV0[0], outHV1[3] + outHV1[0], outHV0, outHV1))
    {
        return false;
    }

    // Clip so that y/w >= -1.
    if (!Clipper(outHV0[3] + outHV0[1], outHV1[3] + outHV1[1], outHV0, outHV1))
    {
        return false;
    }

    // Clip so that z/w >= -1.
    if (!Clipper(outHV0[3] + outHV0[2], outHV1[3] + outHV1[2], outHV0, outHV1))
    {
        return false;
    }

    return true;
}

float DotPerp(in float2 u, in float2 v)
{
    return u.x * v.y - u.y * v.x;
}

bool IsContourEdge(in float4 HV0, in float4 HV1, in float4 HV2, in float4 HV3, in bool hasAdjacent,
    inout float4 HC00, inout float4 HC01, inout float4 HC10, inout float4 HC11)
{
    float4 clipHV0, clipHV1, clipHV2, clipHV3, HMid, clipHMid;
    float2 ndc0, ndc1, ndc2, ndc3, edge10, edge20, edge30;

    if (ClipLine(HV0, HV1, clipHV0, clipHV1))
    {
        // A subtriangle is inside the view frustum. Edge (HV0,HV1) is a
        // contour edge when (1) the subtriangle is visible and (2) the
        // adjacent triangle exists and is not visible or there is no
        // adjacent triangle.
        HMid = (clipHV0 + clipHV1) * 0.5;
        ClipLine(HV2, HMid, clipHV2, clipHMid);
        ndc0 = clipHV0.xy / clipHV0.w;
        ndc1 = clipHV1.xy / clipHV1.w;
        ndc2 = clipHV2.xy / clipHV2.w;
        edge10 = ndc1 - ndc0;
        edge20 = ndc2 - ndc0;
        if (DotPerp(edge10, edge20) > 0)
        {
            if (hasAdjacent)
            {
                // The adjacent triangle exists.
                ClipLine(HV3, HMid, clipHV3, clipHMid);
                ndc3 = clipHV3.xy / clipHV3.w;
                edge30 = ndc3 - ndc0;
                if (DotPerp(edge30, edge10) <= 0)
                {
                    ComputeClipPointsForCorners(clipHV0, clipHV1, clipHV2, HC00, HC01, HC10, HC11);
                    return true;
                }
            }
            else
            {
                // The edge is a boundary edge (no adjacent triangle).
                ComputeClipPointsForCorners(clipHV0, clipHV1, clipHV2, HC00, HC01, HC10, HC11);
                return true;
            }
        }
    }
    return false;
}

[maxvertexcount(12)]
void GSMain(
    uint triangleID : SV_PRIMITIVEID,
    in triangleadj VS_OUTPUT input[6],
    inout TriangleStream<GS_OUTPUT> stream)
{
    uint indices[6];
    float4 HV[6];
    for (int i = 0; i < 6; ++i)
    {
        indices[i] = input[i].vertexID;
        float4 modelPosition = float4(vertices[indices[i]].modelPosition, 1);
#if GTE_USE_MAT_VEC
        HV[i] = mul(pvwMatrix, modelPosition);
#else
        HV[i] = mul(modelPosition, pvwMatrix);
#endif
    }

    float4 HC00 = 0, HC01 = 0, HC10 = 0, HC11 = 0;
    bool hasAdjacent;

    GS_OUTPUT output;
    output.attributes = uint2(meshID, triangleID);

    hasAdjacent = (indices[1] != indices[0] && indices[1] != indices[2]);
    if (IsContourEdge(HV[0], HV[2], HV[4], HV[1], hasAdjacent, HC00, HC01, HC10, HC11))
    {
        output.depth = 1 - saturate(HC00.z / HC00.w);
        output.clipPosition = HC00;
        stream.Append(output);

        output.depth = 1 - saturate(HC10.z / HC10.w);
        output.clipPosition = HC10;
        stream.Append(output);

        output.depth = 1 - saturate(HC01.z / HC01.w);
        output.clipPosition = HC01;
        stream.Append(output);

        output.depth = 1 - saturate(HC11.z / HC11.w);
        output.clipPosition = HC11;
        stream.Append(output);

        stream.RestartStrip();
    }

    hasAdjacent = (indices[3] != indices[2] && indices[3] != indices[4]);
    if (IsContourEdge(HV[2], HV[4], HV[0], HV[3], hasAdjacent, HC00, HC01, HC10, HC11))
    {
        output.depth = 1 - saturate(HC00.z / HC00.w);
        output.clipPosition = HC00;
        stream.Append(output);

        output.depth = 1 - saturate(HC10.z / HC10.w);
        output.clipPosition = HC10;
        stream.Append(output);

        output.depth = 1 - saturate(HC01.z / HC01.w);
        output.clipPosition = HC01;
        stream.Append(output);

        output.depth = 1 - saturate(HC11.z / HC11.w);
        output.clipPosition = HC11;
        stream.Append(output);

        stream.RestartStrip();
    }

    hasAdjacent = (indices[5] != indices[4] && indices[5] != indices[0]);
    if (IsContourEdge(HV[4], HV[0], HV[2], HV[5], hasAdjacent, HC00, HC01, HC10, HC11))
    {
        output.depth = 1 - saturate(HC00.z / HC00.w);
        output.clipPosition = HC00;
        stream.Append(output);

        output.depth = 1 - saturate(HC10.z / HC10.w);
        output.clipPosition = HC10;
        stream.Append(output);

        output.depth = 1 - saturate(HC01.z / HC01.w);
        output.clipPosition = HC01;
        stream.Append(output);

        output.depth = 1 - saturate(HC11.z / HC11.w);
        output.clipPosition = HC11;
        stream.Append(output);

        stream.RestartStrip();
    }
}

// pixel shader
struct PS_OUTPUT
{
    uint4 attributes : SV_TARGET0;
};

PS_OUTPUT PSMain(GS_OUTPUT input)
{
    PS_OUTPUT output;
    output.attributes = uint4(input.attributes.xy, asuint(input.depth), 0);
    return output;
}
