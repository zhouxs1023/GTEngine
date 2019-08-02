// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.1 (2019/04/17)

#include "TerrainEffect.h"
using namespace gte;

TerrainEffect::TerrainEffect(std::shared_ptr<VisualProgram> const& program,
    std::shared_ptr<Texture2> const& baseTexture,
    std::shared_ptr<Texture2> const& detailTexture,
    Vector4<float> const& fogColorDensity)
    :
    mBaseTexture(baseTexture),
    mDetailTexture(detailTexture)
{
    mProgram = program;

    mVWMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);

    mFogColorDensityConstant = std::make_shared<ConstantBuffer>(sizeof(Vector4<float>), false);
    *mFogColorDensityConstant->Get<Vector4<float>>() = fogColorDensity;

    mBaseSampler = std::make_shared<SamplerState>();
    mBaseSampler->filter = SamplerState::MIN_L_MAG_L_MIP_L;
    mBaseSampler->mode[0] = SamplerState::CLAMP;
    mBaseSampler->mode[1] = SamplerState::CLAMP;

    mDetailSampler = std::make_shared<SamplerState>();
    mDetailSampler->filter = SamplerState::MIN_L_MAG_L_MIP_L;
    mDetailSampler->mode[0] = SamplerState::CLAMP;
    mDetailSampler->mode[1] = SamplerState::CLAMP;

    auto vshader = mProgram->GetVShader();
    vshader->Set("PVWMatrix", mPVWMatrixConstant);
    vshader->Set("VWMatrix", mVWMatrixConstant);
    vshader->Set("FogColorDensity", mFogColorDensityConstant);

    auto pshader = mProgram->GetPShader();
    pshader->Set("baseTexture", mBaseTexture, "baseSampler", mBaseSampler);
    pshader->Set("detailTexture", mDetailTexture, "detailSampler", mDetailSampler);
}

void TerrainEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}
