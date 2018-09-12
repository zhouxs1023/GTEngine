// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2018/09/07)

#pragma once

#include <Graphics/GteCamera.h>
#include <Graphics/GteDrawTarget.h>
#include <Graphics/GteVisual.h>
#include <Graphics/GteVisualEffect.h>
#include <Applications/GteEnvironment.h>

namespace gte
{
    class GraphicsEngine;

    class CubeMapEffect : public VisualEffect
    {
    public:
        // Construction.
        CubeMapEffect(std::shared_ptr<ProgramFactory> const& factory, Environment const& environment,
            std::shared_ptr<TextureCube> const& texture, float reflectivity, bool& created);

        // Call this member function after construction if you want to allow
        // dynamic updates of the cube map.  The dmin and dmax values are the
        // desired near and far values for the cube-map camera.
        void UseDynamicUpdates(float dmin, float dmax);

        // For dynamic updating of the cube map.  This function computes the
        // new faces only when UseDynamicUpdates(...) was called after
        // construction.
        inline bool DynamicUpdates() const
        {
            return mDynamicUpdates;
        }

        // Member access.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer);

        inline void SetWMatrix(Matrix4x4<float> const& wMatrix)
        {
            *mWMatrixConstant->Get<Matrix4x4<float>>() = wMatrix;
        }

        inline Matrix4x4<float> const& GetWMatrix() const
        {
            return *mWMatrixConstant->Get<Matrix4x4<float>>();
        }

        inline std::shared_ptr<ConstantBuffer> const& GetWMatrixConstant() const
        {
            return mWMatrixConstant;
        }

        inline void SetCameraWorldPosition(Vector4<float> const& cameraWorldPosition)
        {
            *mCameraWorldPositionConstant->Get<Vector4<float>>() = cameraWorldPosition;
        }

        inline Vector4<float> const& GetCameraWorldPosition() const
        {
            return *mCameraWorldPositionConstant->Get<Vector4<float>>();
        }

        inline std::shared_ptr<ConstantBuffer> const& GetCameraWorldPositionConstant() const
        {
            return mCameraWorldPositionConstant;
        }

        inline void SetReflectivity(float reflectivity)
        {
            *mReflectivityConstant->Get<float>() = reflectivity;
        }

        inline float GetReflectivity() const
        {
            return *mReflectivityConstant->Get<float>();
        }

        inline std::shared_ptr<ConstantBuffer> const& GetReflectivityConstant() const
        {
            return mReflectivityConstant;
        }

        void UpdateFaces(std::shared_ptr<GraphicsEngine> const& engine,
            std::shared_ptr<Spatial> const& scene, Culler& culler,
            Vector4<float> const& envOrigin, Vector4<float> const& envDVector,
            Vector4<float> const& envUVector, Vector4<float> const& envRVector);

    protected:
        // Vertex shader parameters.
        std::shared_ptr<ConstantBuffer> mWMatrixConstant;
        std::shared_ptr<ConstantBuffer> mCameraWorldPositionConstant;

        // Pixel shader parameters.
        std::shared_ptr<ConstantBuffer> mReflectivityConstant;
        std::shared_ptr<TextureCube> mCubeTexture;
        std::shared_ptr<SamplerState> mCubeSampler;

        // Support for dynamic updates of the cube map.
        std::shared_ptr<Camera> mCamera;
        std::shared_ptr<DrawTarget> mTarget;
        bool mDynamicUpdates;
    };
}
