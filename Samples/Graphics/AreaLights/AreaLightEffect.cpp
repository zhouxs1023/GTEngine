// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2018/09/07)

#include <Graphics/GteMaterial.h>
#include "AreaLightEffect.h"
using namespace gte;

AreaLightEffect::AreaLightEffect(std::shared_ptr<ProgramFactory> const& factory,
    Environment const& environment, std::shared_ptr<Texture2> const& baseTexture,
    std::shared_ptr<Texture2> const& normalTexture, bool& created)
{
    created = false;

    // Load and compile the shaders.
#if defined(GTE_DEV_OPENGL)
    std::string pathVS = environment.GetPath("AreaLightVS.glsl");
    std::string pathPS = environment.GetPath("AreaLightPS.glsl");
    mProgram = factory->CreateFromFiles(pathVS, pathPS, "");
#else
    std::string path = environment.GetPath("AreaLight.hlsl");
    mProgram = factory->CreateFromFiles(path, path, "");
#endif

    if (!mProgram)
    {
        // The program factory will generate Log* messages.
        return;
    }

    // Create the shader constants.  These must be initialized by the
    // application before the first use of the effect.
    mMaterialConstant = std::make_shared<ConstantBuffer>(sizeof(Material), true);
    mCameraConstant = std::make_shared<ConstantBuffer>(sizeof(Vector4<float>), true);
    mAreaLightConstant = std::make_shared<ConstantBuffer>(sizeof(Parameters), true);

    // Create the texture sampler for mipmapping.
    mBaseTexture = baseTexture;
    mNormalTexture = normalTexture;
    mCommonSampler = std::make_shared<SamplerState>();
    mCommonSampler->filter = SamplerState::MIN_L_MAG_L_MIP_L;
    mCommonSampler->mode[0] = SamplerState::WRAP;
    mCommonSampler->mode[1] = SamplerState::WRAP;

    // Set the resources for the shaders.
    std::shared_ptr<VertexShader> vshader = mProgram->GetVShader();
    std::shared_ptr<PixelShader> pshader = mProgram->GetPShader();
    vshader->Set("PVWMatrix", mPVWMatrixConstant);
    pshader->Set("Material", mMaterialConstant);
    pshader->Set("Camera", mCameraConstant);
    pshader->Set("AreaLight", mAreaLightConstant);
#if defined(GTE_DEV_OPENGL)
    pshader->Set("baseSampler", mBaseTexture);
    pshader->Set("baseSampler", mCommonSampler);
    pshader->Set("normalSampler", mNormalTexture);
    pshader->Set("normalSampler", mCommonSampler);
#else
    pshader->Set("baseTexture", mBaseTexture);
    pshader->Set("normalTexture", mNormalTexture);
    pshader->Set("commonSampler", mCommonSampler);
#endif

    created = true;
}

void AreaLightEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}
