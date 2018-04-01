// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <Mathematics/GteMatrix4x4.h>
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
    inline void SetPVWMatrix(Matrix4x4<float> const& pvwMatrix);
    inline Matrix4x4<float> const& GetPVWMatrix() const;
    inline void SetVWMatrix(Matrix4x4<float> const& vwMatrix);
    inline Matrix4x4<float> const& GetVWMatrix() const;

    // Required to bind and update resources.
    inline std::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const;
    inline std::shared_ptr<ConstantBuffer> const& GetVWMatrixConstant() const;
    inline std::shared_ptr<Texture2> const& GetTexture() const;
    inline std::shared_ptr<SamplerState> const& GetSampler() const;

private:
    // Vertex shader parameters.
    std::shared_ptr<ConstantBuffer> mPVWMatrixConstant;
    std::shared_ptr<ConstantBuffer> mVWMatrixConstant;

    // Pixel shader parameters.
    std::shared_ptr<Texture2> mTexture;
    std::shared_ptr<SamplerState> mSampler;

    // Convenience pointers.
    Matrix4x4<float>* mPVWMatrix;
    Matrix4x4<float>* mVWMatrix;

    // Shader source code as strings.
    static std::string const msGLSLVSSource;
    static std::string const msGLSLPSSource;
    static std::string const msHLSLSource;
    static std::string const* msVSSource[ProgramFactory::PF_NUM_API];
    static std::string const* msPSSource[ProgramFactory::PF_NUM_API];
};


inline void SphereMapEffect::SetPVWMatrix(Matrix4x4<float> const& pvwMatrix)
{
    *mPVWMatrix = pvwMatrix;
}

inline Matrix4x4<float> const& SphereMapEffect::GetPVWMatrix() const
{
    return *mPVWMatrix;
}

inline void SphereMapEffect::SetVWMatrix(Matrix4x4<float> const& vwMatrix)
{
    *mVWMatrix = vwMatrix;
}

inline Matrix4x4<float> const& SphereMapEffect::GetVWMatrix() const
{
    return *mVWMatrix;
}

inline std::shared_ptr<ConstantBuffer> const& SphereMapEffect::GetPVWMatrixConstant() const
{
    return mPVWMatrixConstant;
}

inline std::shared_ptr<ConstantBuffer> const& SphereMapEffect::GetVWMatrixConstant() const
{
    return mVWMatrixConstant;
}

inline std::shared_ptr<Texture2> const& SphereMapEffect::GetTexture() const
{
    return mTexture;
}

inline std::shared_ptr<SamplerState> const& SphereMapEffect::GetSampler() const
{
    return mSampler;
}

}
