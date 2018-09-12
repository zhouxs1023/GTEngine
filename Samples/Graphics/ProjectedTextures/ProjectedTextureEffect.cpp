// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2018/09/07)

#include "ProjectedTextureEffect.h"
using namespace gte;

ProjectedTextureEffect::ProjectedTextureEffect(
    std::shared_ptr<ProgramFactory> const& factory, BufferUpdater const& updater,
    std::shared_ptr<Material> const& material, std::shared_ptr<Lighting> const& lighting,
    std::shared_ptr<LightCameraGeometry> const& geometry,
    std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
    SamplerState::Mode mode0, SamplerState::Mode mode1)
    :
    LightingEffect(factory, updater, msVSSource, msPSSource, material, lighting, geometry),
    mTexture(texture)
{
    mSampler = std::make_shared<SamplerState>();
    mSampler->filter = filter;
    mSampler->mode[0] = mode0;
    mSampler->mode[1] = mode1;

    mMaterialConstant = std::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
    UpdateMaterialConstant();

    mLightingConstant = std::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
    UpdateLightingConstant();

    mGeometryConstant = std::make_shared<ConstantBuffer>(sizeof(InternalGeometry), true);
    UpdateGeometryConstant();

    mProjectorMatrixConstant =
        std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);

    if (mProgram)
    {
        mProgram->GetVShader()->Set("ProjectorMatrix", mProjectorMatrixConstant);
        mProgram->GetVShader()->Set("Material", mMaterialConstant);
        mProgram->GetVShader()->Set("Lighting", mLightingConstant);
        mProgram->GetVShader()->Set("LightCameraGeometry", mGeometryConstant);
#if defined(GTE_DEV_OPENGL)
        mProgram->GetPShader()->Set("baseSampler", mTexture);
#else
        mProgram->GetPShader()->Set("baseTexture", mTexture);
#endif
        mProgram->GetPShader()->Set("baseSampler", mSampler);
    }
}

void ProjectedTextureEffect::SetProjectorMatrix(Matrix4x4<float> const& projectorMatrix)
{
    *mProjectorMatrixConstant->Get<Matrix4x4<float>>() = projectorMatrix;
}

void ProjectedTextureEffect::UpdateMaterialConstant()
{
    InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
    internalMaterial->emissive = mMaterial->emissive;
    internalMaterial->ambient = mMaterial->ambient;
    internalMaterial->diffuse = mMaterial->diffuse;
    internalMaterial->specular = mMaterial->specular;
    LightingEffect::UpdateMaterialConstant();
}

void ProjectedTextureEffect::UpdateLightingConstant()
{
    InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
    internalLighting->ambient = mLighting->ambient;
    internalLighting->diffuse = mLighting->diffuse;
    internalLighting->specular = mLighting->specular;
    internalLighting->attenuation = mLighting->attenuation;
    LightingEffect::UpdateLightingConstant();
}

void ProjectedTextureEffect::UpdateGeometryConstant()
{
    InternalGeometry* internalGeometry = mGeometryConstant->Get<InternalGeometry>();
    internalGeometry->lightModelDirection = mGeometry->lightModelDirection;
    internalGeometry->cameraModelPosition = mGeometry->cameraModelPosition;
    LightingEffect::UpdateGeometryConstant();
}

void ProjectedTextureEffect::UpdateProjectorMatrixConstant()
{
    mBufferUpdater(mProjectorMatrixConstant);
}

std::string const ProjectedTextureEffect::msGLSLVSSource =
LightingEffect::GetGLSLLitFunction() +
"uniform PVWMatrix\n"
"{\n"
"    mat4 pvwMatrix;\n"
"};\n"
"\n"
"uniform ProjectorMatrix\n"
"{\n"
"    mat4 projectorMatrix;\n"
"};\n"
"\n"
"uniform Material\n"
"{\n"
"    vec4 materialEmissive;\n"
"    vec4 materialAmbient;\n"
"    vec4 materialDiffuse;\n"
"    vec4 materialSpecular;\n"
"};\n"
"\n"
"uniform Lighting\n"
"{\n"
"    vec4 lightingAmbient;\n"
"    vec4 lightingDiffuse;\n"
"    vec4 lightingSpecular;\n"
"    vec4 lightingAttenuation;\n"
"};\n"
"\n"
"uniform LightCameraGeometry\n"
"{\n"
"    vec4 lightModelDirection;\n"
"    vec4 cameraModelPosition;\n"
"};\n"
"\n"
"layout(location = 0) in vec3 inModelPosition;\n"
"layout(location = 1) in vec3 inModelNormal;\n"
"\n"
"layout(location = 0) out vec4 vertexColor;\n"
"layout(location = 1) out vec4 projectorTCoord;\n"
"\n"
"void main()\n"
"{\n"
"    float NDotL = -dot(inModelNormal, lightModelDirection.xyz);\n"
"    vec3 viewVector = normalize(cameraModelPosition.xyz - inModelPosition);\n"
"    vec3 halfVector = normalize(viewVector - lightModelDirection.xyz);\n"
"    float NDotH = dot(inModelNormal, halfVector);\n"
"    vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);\n"
"\n"
"    vertexColor.rgb = materialEmissive.rgb +\n"
"        materialAmbient.rgb * lightingAmbient.rgb +\n"
"        lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +\n"
"        lighting.z * materialSpecular.rgb * lightingSpecular.rgb;\n"
"    vertexColor.a = materialDiffuse.a;\n"
"\n"
"    vec4 modelPosition = vec4(inModelPosition, 1.0f);\n"
"#if GTE_USE_MAT_VEC\n"
"    gl_Position = pvwMatrix * modelPosition;\n"
"    projectorTCoord = projectorMatrix * modelPosition;\n"
"#else\n"
"    gl_Position = modelPosition * pvwMatrix;\n"
"    projectorTCoord = modelPosition * projectorMatrix;\n"
"#endif\n"
"}\n";

std::string const ProjectedTextureEffect::msGLSLPSSource =
"layout(location = 0) in vec4 vertexColor;\n"
"layout(location = 1) in vec4 projectorTCoord;\n"
"\n"
"layout(location = 0) out vec4 pixelColor;\n"
"\n"
"uniform sampler2D baseSampler;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 tcoord = projectorTCoord.xy / projectorTCoord.w;\n"
"    vec4 baseColor = texture(baseSampler, tcoord);\n"
"    pixelColor = baseColor * vertexColor;\n"
"}\n";


std::string const ProjectedTextureEffect::msHLSLSource =
"cbuffer PVWMatrix\n"
"{\n"
"    float4x4 pvwMatrix;\n"
"};\n"
"\n"
"cbuffer ProjectorMatrix\n"
"{\n"
"    float4x4 projectorMatrix;\n"
"};\n"
"\n"
"cbuffer Material\n"
"{\n"
"    float4 materialEmissive;\n"
"    float4 materialAmbient;\n"
"    float4 materialDiffuse;\n"
"    float4 materialSpecular;\n"
"};\n"
"\n"
"cbuffer Lighting\n"
"{\n"
"    float4 lightingAmbient;\n"
"    float4 lightingDiffuse;\n"
"    float4 lightingSpecular;\n"
"    float4 lightingAttenuation;\n"
"};\n"
"\n"
"cbuffer LightCameraGeometry\n"
"{\n"
"    float4 lightModelDirection;\n"
"    float4 cameraModelPosition;\n"
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
"    float4 vertexColor : COLOR;\n"
"    float4 projectorTCoord : TEXCOORD0;\n"
"    float4 clipPosition : SV_POSITION;\n"
"};\n"
"\n"
"VS_OUTPUT VSMain(VS_INPUT input)\n"
"{\n"
"    VS_OUTPUT output;\n"
"\n"
"    float NDotL = -dot(input.modelNormal, lightModelDirection.xyz);\n"
"    float3 viewVector = normalize(cameraModelPosition.xyz - input.modelPosition);\n"
"    float3 halfVector = normalize(viewVector - lightModelDirection.xyz);\n"
"    float NDotH = dot(input.modelNormal, halfVector);\n"
"    float4 lighting = lit(NDotL, NDotH, materialSpecular.a);\n"
"\n"
"    output.vertexColor.rgb = materialEmissive.rgb +\n"
"        materialAmbient.rgb * lightingAmbient.rgb +\n"
"        lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +\n"
"        lighting.z * materialSpecular.rgb * lightingSpecular.rgb;\n"
"    output.vertexColor.a = materialDiffuse.a;\n"
"\n"
"    float4 modelPosition = float4(input.modelPosition, 1.0f);\n"
"#if GTE_USE_MAT_VEC\n"
"    output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));\n"
"    output.projectorTCoord = mul(projectorMatrix, modelPosition);\n"
"#else\n"
"    output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);\n"
"    output.projectorTCoord = mul(modelPosition, projectorMatrix);\n"
"#endif\n"
"    return output;\n"
"}\n"
"\n"
"struct PS_INPUT\n"
"{\n"
"    float4 vertexColor : COLOR;\n"
"    float4 projectorTCoord : TEXCOORD0;\n"
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
"    float2 tcoord = input.projectorTCoord.xy / input.projectorTCoord.w;\n"
"    float4 baseColor = baseTexture.Sample(baseSampler, tcoord);\n"
"    output.pixelColor = baseColor * input.vertexColor;\n"
"    return output;\n"
"}\n";

std::string const* ProjectedTextureEffect::msVSSource[ProgramFactory::PF_NUM_API] =
{
    &msGLSLVSSource,
    &msHLSLSource
};

std::string const* ProjectedTextureEffect::msPSSource[ProgramFactory::PF_NUM_API] =
{
    &msGLSLPSSource,
    &msHLSLSource
};
