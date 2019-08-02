// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.2 (2019/04/15)

#include "AreaLightEffect.h"
#include <Graphics/GteGraphicsDefaults.h>
#include <Graphics/GteMaterial.h>
using namespace gte;

AreaLightEffect::AreaLightEffect(std::shared_ptr<ProgramFactory> const& factory,
    Environment const& environment, std::shared_ptr<Texture2> const& baseTexture,
    std::shared_ptr<Texture2> const& normalTexture, bool& created)
{
    created = false;

    // Load and compile the shaders.
    std::string vsPath = environment.GetPath(DefaultShaderName("AreaLight.vs"));
    std::string psPath = environment.GetPath(DefaultShaderName("AreaLight.ps"));
    mProgram = factory->CreateFromFiles(vsPath, psPath, "");
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
    pshader->Set("baseTexture", mBaseTexture, "baseSampler", mCommonSampler);
    pshader->Set("normalTexture", mNormalTexture, "normalSampler", mCommonSampler);

    created = true;
}

void AreaLightEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}
