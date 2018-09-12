// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2018/09/07)

#pragma once

#include <Applications/GteEnvironment.h>
#include <Graphics/GteVisualEffect.h>

namespace gte
{
    class AreaLightEffect : public VisualEffect
    {
    public:
        struct Parameters
        {
            Vector4<float> ambient;
            Vector4<float> diffuse;
            Vector4<float> specular;
            Vector4<float> attenuation;
            Vector4<float> position;    // (x,y,z,1)
            Vector4<float> normal;      // (x,y,z,0)
            Vector4<float> axis0;       // (x,y,z,0)
            Vector4<float> axis1;       // (x,y,z,0)
            Vector4<float> extent;      // (extent0, extent1, *, *)
        };

        // Construction.  The shader constants are not initialized by the
        // constructor.  They must be initialized by the application before the
        // first use of the effect.
        AreaLightEffect(std::shared_ptr<ProgramFactory> const& factory,
            Environment const& environment, std::shared_ptr<Texture2> const& baseTexture,
            std::shared_ptr<Texture2> const& normalTexture, bool& created);

        // Required to bind and update resources.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer);

        inline std::shared_ptr<ConstantBuffer> const& GetMaterialConstant() const
        {
            return mMaterialConstant;
        }

        inline std::shared_ptr<ConstantBuffer> const& GetCameraConstant() const
        {
            return mCameraConstant;
        }

        inline std::shared_ptr<ConstantBuffer> const& GetAreaLightConstant() const
        {
            return mAreaLightConstant;
        }

    private:
        // Pixel shader parameters.
        std::shared_ptr<ConstantBuffer> mMaterialConstant;
        std::shared_ptr<ConstantBuffer> mCameraConstant;
        std::shared_ptr<ConstantBuffer> mAreaLightConstant;
        std::shared_ptr<Texture2> mBaseTexture, mNormalTexture;
        std::shared_ptr<SamplerState> mCommonSampler;
    };
}
