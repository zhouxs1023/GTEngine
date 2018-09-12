// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2018/09/07)

#include "SphereMapEffect.h"
using namespace gte;

SphereMapEffect::SphereMapEffect(std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
    SamplerState::Mode mode0, SamplerState::Mode mode1)
    :
    mTexture(texture)
{
    int api = factory->GetAPI();
    mProgram = factory->CreateFromSources(*msVSSource[api], *msPSSource[api], "");
    if (mProgram)
    {
        mVWMatrixConstant = std::make_shared<ConstantBuffer>(
            sizeof(Matrix4x4<float>), true);
        *mVWMatrixConstant->Get<Matrix4x4<float>>() = Matrix4x4<float>::Identity();

        mSampler = std::make_shared<SamplerState>();
        mSampler->filter = filter;
        mSampler->mode[0] = mode0;
        mSampler->mode[1] = mode1;

        mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
        mProgram->GetVShader()->Set("VWMatrix", mVWMatrixConstant);
#if defined(GTE_DEV_OPENGL)
        mProgram->GetPShader()->Set("baseSampler", texture);
#else
        mProgram->GetPShader()->Set("baseTexture", texture);
#endif
        mProgram->GetPShader()->Set("baseSampler", mSampler);
    }
}

void SphereMapEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

std::string const SphereMapEffect::msGLSLVSSource =
"uniform PVWMatrix\n"
"{\n"
"    mat4 pvwMatrix;\n"
"};\n"
"\n"
"uniform VWMatrix\n"
"{\n"
"    mat4 vwMatrix;\n"
"};\n"
"\n"
"layout(location = 0) in vec3 inModelPosition;\n"
"layout(location = 1) in vec3 inModelNormal;\n"
"\n"
"layout(location = 0) out vec2 vertexTCoord;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 modelPosition = vec4(inModelPosition, 1.0f);\n"
"    vec4 modelNormal = vec4(inModelNormal, 0.0f);\n"
"\n"
"#if GTE_USE_MAT_VEC\n"
"    vec4 cameraSpacePosition = vwMatrix * modelPosition;\n"
"    vec3 cameraSpaceNormal = normalize((vwMatrix * modelNormal).xyz);\n"
"    gl_Position = pvwMatrix * modelPosition;\n"
"#else\n"
"    vec4 cameraSpacePosition = modelPosition * vwMatrix;\n"
"    vec3 cameraSpaceNormal = normalize((modelNormal * vwMatrix).xyz);\n"
"    gl_Position = modelPosition * pvwMatrix;\n"
"#endif\n"
"\n"
"    vec3 eyeDirection = normalize(cameraSpacePosition.xyz);\n"
"    vec3 r = reflect(eyeDirection, cameraSpaceNormal);\n"
"\n"
"    float oneMRZ = 1.0f - r.z;\n"
"    float invLength = 1.0f / sqrt(r.x * r.x + r.y * r.y + oneMRZ * oneMRZ);\n"
"    vertexTCoord = 0.5f * (r.xy * invLength + 1.0f);\n"
"}\n";

std::string const SphereMapEffect::msGLSLPSSource =
"layout(location = 0) in vec2 vertexTCoord;\n"
"\n"
"layout(location = 0) out vec4 pixelColor;\n"
"\n"
"uniform sampler2D baseSampler;\n"
"\n"
"void main()\n"
"{\n"
"    pixelColor = texture(baseSampler, vertexTCoord);\n"
"}\n";


std::string const SphereMapEffect::msHLSLSource =
"cbuffer PVWMatrix\n"
"{\n"
"    float4x4 pvwMatrix;\n"
"};\n"
"\n"
"cbuffer VWMatrix\n"
"{\n"
"    float4x4 vwMatrix;\n"
"};\n"
"\n"
"struct VS_INPUT\n"
"{\n"
"    float3 modelPosition : POSITION;\n"
"    float3 modelNormal : NORMAL;\n"
"};\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"    float2 vertexTCoord : TEXCOORD0;\n"
"    float4 clipPosition : SV_POSITION;\n"
"};\n"
"\n"
"VS_OUTPUT VSMain(VS_INPUT input)\n"
"{\n"
"    VS_OUTPUT output;\n"
"\n"
"    float4 modelPosition = float4(input.modelPosition, 1.0f);\n"
"    float4 modelNormal = float4(input.modelNormal, 0.0f);\n"
"\n"
"#if GTE_USE_MAT_VEC\n"
"    float4 cameraSpacePosition = mul(vwMatrix, modelPosition);\n"
"    float3 cameraSpaceNormal = normalize(mul(vwMatrix, modelNormal).xyz);\n"
"    output.clipPosition = mul(pvwMatrix, modelPosition);\n"
"#else\n"
"    float4 cameraSpacePosition = mul(modelPosition, vwMatrix);\n"
"    float3 cameraSpaceNormal = normalize(mul(modelNormal, vwMatrix).xyz);\n"
"    output.clipPosition = mul(modelPosition, pvwMatrix);\n"
"#endif\n"
"\n"
"    float3 eyeDirection = normalize(cameraSpacePosition.xyz);\n"
"    float3 r = reflect(eyeDirection, cameraSpaceNormal);\n"
"\n"
"    float oneMRZ = 1.0f - r.z;\n"
"    float invLength = 1.0f / sqrt(r.x * r.x + r.y * r.y + oneMRZ * oneMRZ);\n"
"    output.vertexTCoord = 0.5f * (r.xy * invLength + 1.0f);\n"
"\n"
"    return output;\n"
"}\n"
"\n"
"struct PS_INPUT\n"
"{\n"
"    float2 vertexTCoord : TEXCOORD0;\n"
"};\n"
"\n"
"struct PS_OUTPUT\n"
"{\n"
"    float4 pixelColor : SV_TARGET0;\n"
"};\n"
"\n"
"Texture2D<float4> baseTexture;\n"
"SamplerState baseSampler;\n"
"\n"
"PS_OUTPUT PSMain(PS_INPUT input)\n"
"{\n"
"    PS_OUTPUT output;\n"
"    output.pixelColor = baseTexture.Sample(baseSampler, input.vertexTCoord);\n"
"    return output;\n"
"}\n";

std::string const* SphereMapEffect::msVSSource[] =
{
    &msGLSLVSSource,
    &msHLSLSource
};

std::string const* SphereMapEffect::msPSSource[] =
{
    &msGLSLPSSource,
    &msHLSLSource
};
