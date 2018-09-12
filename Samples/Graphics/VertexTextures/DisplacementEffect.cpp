// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2018/09/07)

#include "DisplacementEffect.h"
using namespace gte;

DisplacementEffect::DisplacementEffect(std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
    SamplerState::Mode mode0, SamplerState::Mode mode1)
    :
    mTexture(texture)
{
    int api = factory->GetAPI();
    mProgram = factory->CreateFromSources(*msVSSource[api], *msPSSource[api], "");
    if (mProgram)
    {
        mSampler = std::make_shared<SamplerState>();
        mSampler->filter = filter;
        mSampler->mode[0] = mode0;
        mSampler->mode[1] = mode1;

        mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
#if defined(GTE_DEV_OPENGL)
        mProgram->GetVShader()->Set("displacementSampler", texture);
#else
        mProgram->GetVShader()->Set("displacementTexture", texture);
#endif
        mProgram->GetVShader()->Set("displacementSampler", mSampler);
    }
}

void DisplacementEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}


std::string const DisplacementEffect::msGLSLVSSource =
"uniform PVWMatrix\n"
"{\n"
"    mat4 pvwMatrix;\n"
"};\n"
"\n"
"layout(location = 0) in vec3 modelPosition;\n"
"layout(location = 1) in vec2 modelTCoord;\n"
"\n"
"layout(location = 0) out float vertexHeight;\n"
"\n"
"uniform sampler2D displacementSampler;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 displacedPosition;\n"
"    displacedPosition.xy = modelPosition.xy;\n"
"    displacedPosition.z = textureLod(displacementSampler, modelTCoord, 0).x;\n"
"    displacedPosition.w = 1.0f;\n"
"\n"
"    vertexHeight = displacedPosition.z;\n"
"\n"
"#if GTE_USE_MAT_VEC\n"
"    gl_Position = pvwMatrix * displacedPosition;\n"
"#else\n"
"    gl_Position = displacedPosition * pvwMatrix;\n"
"#endif\n"
"}\n";

std::string const DisplacementEffect::msGLSLPSSource =
"layout(location = 0) in float vertexHeight;\n"
"\n"
"layout(location = 0) out vec4 pixelColor;\n"
"\n"
"void main()\n"
"{\n"
"    pixelColor.rgb = vec3(vertexHeight);\n"
"    pixelColor.a = 1.0f;\n"
"}\n";


std::string const DisplacementEffect::msHLSLSource =
"cbuffer PVWMatrix\n"
"{\n"
"    float4x4 pvwMatrix;\n"
"};\n"
"\n"
"struct VS_INPUT\n"
"{\n"
"    float3 modelPosition : POSITION;\n"
"    float2 modelTCoord : TEXCOORD0;\n"
"};\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"    float vertexHeight : TEXCOORD0;\n"
"    float4 clipPosition : SV_POSITION;\n"
"};\n"
"\n"
"Texture2D<float> displacementTexture;\n"
"SamplerState displacementSampler;\n"
"\n"
"VS_OUTPUT VSMain(VS_INPUT input)\n"
"{\n"
"    VS_OUTPUT output;\n"
"\n"
"    float4 displacedPosition;\n"
"    displacedPosition.xy = input.modelPosition.xy;\n"
"    displacedPosition.z = displacementTexture.SampleLevel(\n"
"        displacementSampler, input.modelTCoord, 0);\n"
"    displacedPosition.w = 1.0f;\n"
"\n"
"    output.vertexHeight = displacedPosition.z;\n"
"\n"
"#if GTE_USE_MAT_VEC\n"
"    output.clipPosition = mul(pvwMatrix, displacedPosition);\n"
"#else\n"
"    output.clipPosition = mul(displacedPosition, pvwMatrix);\n"
"#endif\n"
"\n"
"    return output;\n"
"}\n"
"\n"
"struct PS_INPUT\n"
"{\n"
"    float vertexHeight : TEXCOORD0;\n"
"};\n"
"\n"
"struct PS_OUTPUT\n"
"{\n"
"    float4 pixelColor : SV_TARGET0;\n"
"};\n"
"\n"
"PS_OUTPUT PSMain(PS_INPUT input)\n"
"{\n"
"    PS_OUTPUT output;\n"
"    output.pixelColor.rgb = input.vertexHeight;\n"
"    output.pixelColor.a = 1.0f;\n"
"    return output;\n"
"}\n";

std::string const* DisplacementEffect::msVSSource[] =
{
    &msGLSLVSSource,
    &msHLSLSource
};

std::string const* DisplacementEffect::msPSSource[] =
{
    &msGLSLPSSource,
    &msHLSLSource
};
