// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2018/09/07)

#pragma once

#include <Graphics/GteVisualEffect.h>

namespace gte
{
    class SphereMapEffect : public VisualEffect
    {
    public:
        // Construction.
        SphereMapEffect(std::shared_ptr<ProgramFactory> const& factory,
            std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
            SamplerState::Mode mode0, SamplerState::Mode mode1);

        // Member access.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer);

        inline void SetVWMatrix(Matrix4x4<float> const& vwMatrix)
        {
            *mVWMatrixConstant->Get<Matrix4x4<float>>() = vwMatrix;
        }

        inline Matrix4x4<float> const& GetVWMatrix() const
        {
            return *mVWMatrixConstant->Get<Matrix4x4<float>>();
        }

        inline std::shared_ptr<ConstantBuffer> const& GetVWMatrixConstant() const
        {
            return mVWMatrixConstant;
        }

        inline std::shared_ptr<Texture2> const& GetTexture() const
        {
            return mTexture;
        }

        inline std::shared_ptr<SamplerState> const& GetSampler() const
        {
            return mSampler;
        }

    private:
        // Vertex shader parameter.
        std::shared_ptr<ConstantBuffer> mVWMatrixConstant;

        // Pixel shader parameters.
        std::shared_ptr<Texture2> mTexture;
        std::shared_ptr<SamplerState> mSampler;

        // Shader source code as strings.
        static std::string const msGLSLVSSource;
        static std::string const msGLSLPSSource;
        static std::string const msHLSLSource;
        static std::string const* msVSSource[ProgramFactory::PF_NUM_API];
        static std::string const* msPSSource[ProgramFactory::PF_NUM_API];
    };
}
