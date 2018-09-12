// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2018/09/07)

#pragma once

#include <Mathematics/GteVector2.h>
#include <Graphics/GteTexture2.h>
#include <Graphics/GteVisual.h>
#include <Graphics/GteVisualEffect.h>
#include <Applications/GteEnvironment.h>

namespace gte
{
    class SimpleBumpMapEffect : public VisualEffect
    {
    public:
        // Construction.
        SimpleBumpMapEffect(std::shared_ptr<ProgramFactory> const& factory,
            Environment const& environment, bool& created);

        // Member access.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer);

        inline std::shared_ptr<Texture2> const& GetBaseTexture() const
        {
            return mBaseTexture;
        }

        inline std::shared_ptr<Texture2> const& GetNormalTexture() const
        {
            return mNormalTexture;
        }

        inline std::shared_ptr<SamplerState> const& GetCommonSampler() const
        {
            return mCommonSampler;
        }

        // The 'mesh' is one to which an instance of this effect is attached.
        // TODO: Move this into a compute shader to improve performance.
        static void ComputeLightVectors(std::shared_ptr<Visual> const& mesh,
            Vector4<float> const& worldLightDirection);

    private:
        // Compute a tangent at the vertex P0.  The triangle is
        // counterclockwise ordered, <P0,P1,P2>.
        static bool ComputeTangent(
            Vector3<float> const& position0, Vector2<float> const& tcoord0,
            Vector3<float> const& position1, Vector2<float> const& tcoord1,
            Vector3<float> const& position2, Vector2<float> const& tcoord2,
            Vector3<float>& tangent);

        // Pixel shader parameters.
        std::shared_ptr<Texture2> mBaseTexture;
        std::shared_ptr<Texture2> mNormalTexture;
        std::shared_ptr<SamplerState> mCommonSampler;
    };
}
