// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#include <Applications/GteTextureIO.h>
#include "BlendedTerrainEffect.h"
using namespace gte;

BlendedTerrainEffect::BlendedTerrainEffect(std::shared_ptr<ProgramFactory> const& factory,
    Environment const& environment, bool& created)
    :
    mFlowDirection(nullptr),
    mPowerFactor(nullptr)
{
    created = false;

    int api = factory->GetAPI();

    // Load and compile the shaders.
#if defined(GTE_DEV_OPENGL)
    auto pathVertexShader = environment.GetPath("BlendedTerrainVertex.glsl");
    auto pathPixelShader = environment.GetPath("BlendedTerrainPixel.glsl");
    mProgram = factory->CreateFromFiles(pathVertexShader, pathPixelShader, "");
#else
    auto hlslpath = environment.GetPath("BlendedTerrain.hlsl");

    // The flags are chosen to allow you to debug the shaders through MSVS.
    // The menu path is "Debug | Graphics | Start Diagnostics" (ALT+F5).
    factory->PushFlags();
    factory->flags =
        D3DCOMPILE_ENABLE_STRICTNESS |
        D3DCOMPILE_IEEE_STRICTNESS |
        D3DCOMPILE_DEBUG |
        D3DCOMPILE_SKIP_OPTIMIZATION;
    mProgram = factory->CreateFromFiles(hlslpath, hlslpath, "");
    factory->PopFlags();
#endif

    if (!mProgram)
    {
        // The program factory will generate Log* messages.
        return;
    }

    // Load the textures.
    auto path = environment.GetPath("BTGrass.png");
    mGrassTexture = WICFileIO::Load(path, true);
    mGrassTexture->AutogenerateMipmaps();

    path = environment.GetPath("BTStone.png");
    mStoneTexture = WICFileIO::Load(path, true);
    mStoneTexture->AutogenerateMipmaps();

    path = environment.GetPath("BTCloud.png");
    mCloudTexture = WICFileIO::Load(path, true);
    mCloudTexture->AutogenerateMipmaps();

    // Create the shader constants.
    mFlowDirectionConstant = std::make_shared<ConstantBuffer>(sizeof(Vector2<float>), true);
    mFlowDirection = mFlowDirectionConstant->Get<Vector2<float>>();
    *mFlowDirection = { 0.0f, 0.0f };

    mPowerFactorConstant = std::make_shared<ConstantBuffer>(sizeof(float), true);
    mPowerFactor = mPowerFactorConstant->Get<float>();
    *mPowerFactor = 1.0f;

    // Create a 1-dimensional texture whose intensities are proportional to
    // height.
    unsigned int const numTexels = 256;
    mBlendTexture = std::make_shared<Texture1>(DF_R8_UNORM, numTexels);
    unsigned char* texels = mBlendTexture->Get<unsigned char>();
    for (unsigned int i = 0; i < numTexels; ++i, ++texels)
    {
        *texels = static_cast<unsigned char>(i);
    }

    // Create the texture samplers.  The common sampler uses trilinear
    // interpolation (mipmapping).  The blend sample uses bilinear
    // interpolation (no mipmapping).
    mCommonSampler = std::make_shared<SamplerState>();
    mCommonSampler->filter = SamplerState::MIN_L_MAG_L_MIP_L;
    mCommonSampler->mode[0] = SamplerState::WRAP;
    mCommonSampler->mode[1] = SamplerState::WRAP;
    mBlendSampler = std::make_shared<SamplerState>();
    mBlendSampler->filter = SamplerState::MIN_L_MAG_L_MIP_P;
    mBlendSampler->mode[0] = SamplerState::WRAP;

    // Set the resources for the shaders.
    std::shared_ptr<VertexShader> vshader = mProgram->GetVShader();
    std::shared_ptr<PixelShader> pshader = mProgram->GetPShader();
    vshader->Set("PVWMatrix", mPVWMatrixConstant);
    vshader->Set("FlowDirection", mFlowDirectionConstant);
    pshader->Set("PowerFactor", mPowerFactorConstant);
    if (ProgramFactory::PF_GLSL == api)
    {
        pshader->Set("grassSampler", mGrassTexture);
        pshader->Set("grassSampler", mCommonSampler);

        pshader->Set("stoneSampler", mStoneTexture);
        pshader->Set("stoneSampler", mCommonSampler);

        pshader->Set("blendSampler", mBlendTexture);
        pshader->Set("blendSampler", mBlendSampler);

        pshader->Set("cloudSampler", mCloudTexture);
        pshader->Set("cloudSampler", mCommonSampler);
    }
    else if (ProgramFactory::PF_HLSL == api)
    {
        pshader->Set("grassTexture", mGrassTexture);
        pshader->Set("stoneTexture", mStoneTexture);
        pshader->Set("blendTexture", mBlendTexture);
        pshader->Set("cloudTexture", mCloudTexture);
        pshader->Set("commonSampler", mCommonSampler);
        pshader->Set("blendSampler", mBlendSampler);
    }
    else
    {
        LogError("Missing shader texture assignments for API=" + api);
    }

    created = true;
}

void BlendedTerrainEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}
