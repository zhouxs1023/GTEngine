// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.22.0 (2019/01/31)

#pragma once

#include <Graphics/GteTexture2.h>
#include <Graphics/GteVisualEffect.h>

namespace gte
{
    class TerrainEffect : public VisualEffect
    {
    public:
        // Construction.
        TerrainEffect(std::shared_ptr<VisualProgram> const& program,
            std::shared_ptr<Texture2> const& baseTexture,
            std::shared_ptr<Texture2> const& detailTexture,
            Vector4<float> const& fogColorDensity);

        // Member access.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer) override;

        inline std::shared_ptr<ConstantBuffer> const& GetVWMatrixConstant() const
        {
            return mVWMatrixConstant;
        }

        inline std::shared_ptr<ConstantBuffer> const& GetFogColorDensityConstant() const
        {
            return mFogColorDensityConstant;
        }

        inline std::shared_ptr<Texture2> const& GetBaseTexture() const
        {
            return mBaseTexture;
        }

        inline std::shared_ptr<Texture2> const& GetDetailTexture() const
        {
            return mDetailTexture;
        }

        inline std::shared_ptr<SamplerState> const& GetBaseSampler() const
        {
            return mBaseSampler;
        }

        inline std::shared_ptr<SamplerState> const& GetDetailSampler() const
        {
            return mBaseSampler;
        }

    private:
        // Vertex shader parameters.
        std::shared_ptr<ConstantBuffer> mVWMatrixConstant;
        std::shared_ptr<ConstantBuffer> mFogColorDensityConstant;

        // Pixel shader parameters.
        std::shared_ptr<Texture2> mBaseTexture;
        std::shared_ptr<Texture2> mDetailTexture;
        std::shared_ptr<SamplerState> mBaseSampler;
        std::shared_ptr<SamplerState> mDetailSampler;
    };
}
