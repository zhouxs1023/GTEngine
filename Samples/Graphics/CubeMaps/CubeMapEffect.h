// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

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

    // For dynamic updating of the cube map.  This function computes the new
    // faces only when UseDynamicUpdates(...) was called after construction.
    inline bool DynamicUpdates() const;

    // Member access.
    inline void SetPVWMatrix(Matrix4x4<float> const& pvwMatrix);
    inline Matrix4x4<float> const& GetPVWMatrix() const;
    inline void SetWMatrix(Matrix4x4<float> const& wMatrix);
    inline Matrix4x4<float> const& GetWMatrix() const;
    inline void SetCameraWorldPosition(Vector4<float> const& cameraWorldPosition);
    inline Vector4<float> const& GetCameraWorldPosition() const;
    inline void SetReflectivity(float reflectivity);
    inline float GetReflectivity() const;

    // Required to bind and update resources.
    inline std::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const;
    inline std::shared_ptr<ConstantBuffer> const& GetWMatrixConstant() const;
    inline std::shared_ptr<ConstantBuffer> const& GetCameraWorldPositionConstant() const;
    inline std::shared_ptr<ConstantBuffer> const& GetReflectivityConstant() const;

    void UpdateFaces(std::shared_ptr<GraphicsEngine> const& engine,
        std::shared_ptr<Spatial> const& scene, Culler& culler,
        Vector4<float> const& envOrigin, Vector4<float> const& envDVector,
        Vector4<float> const& envUVector, Vector4<float> const& envRVector);

protected:
    // Vertex shader parameters.
    std::shared_ptr<ConstantBuffer> mPVWMatrixConstant;
    std::shared_ptr<ConstantBuffer> mWMatrixConstant;
    std::shared_ptr<ConstantBuffer> mCameraWorldPositionConstant;

    // Pixel shader parameters.
    std::shared_ptr<ConstantBuffer> mReflectivityConstant;
    std::shared_ptr<TextureCube> mCubeTexture;
    std::shared_ptr<SamplerState> mCubeSampler;

    // Convenience pointers.
    Matrix4x4<float>* mPVWMatrix;
    Matrix4x4<float>* mWMatrix;
    Vector4<float>* mCameraWorldPosition;
    float* mReflectivity;

    // Support for dynamic updates of the cube map.
    std::shared_ptr<Camera> mCamera;
    std::shared_ptr<DrawTarget> mTarget;
    bool mDynamicUpdates;
};

inline bool CubeMapEffect::DynamicUpdates() const
{
    return mDynamicUpdates;
}

inline void CubeMapEffect::SetPVWMatrix(Matrix4x4<float> const& pvwMatrix)
{
    *mPVWMatrix = pvwMatrix;
}

inline Matrix4x4<float> const& CubeMapEffect::GetPVWMatrix() const
{
    return *mPVWMatrix;
}

inline void CubeMapEffect::SetWMatrix(Matrix4x4<float> const& wMatrix)
{
    *mWMatrix = wMatrix;
}

inline Matrix4x4<float> const& CubeMapEffect::GetWMatrix() const
{
    return *mWMatrix;
}

inline void CubeMapEffect::SetCameraWorldPosition(Vector4<float> const& cameraWorldPosition)
{
    *mCameraWorldPosition = cameraWorldPosition;
}

inline Vector4<float> const& CubeMapEffect::GetCameraWorldPosition() const
{
    return *mCameraWorldPosition;
}

inline void CubeMapEffect::SetReflectivity(float reflectivity)
{
    *mReflectivity = reflectivity;
}

inline float CubeMapEffect::GetReflectivity() const
{
    return *mReflectivity;
}

inline std::shared_ptr<ConstantBuffer> const& CubeMapEffect::GetPVWMatrixConstant() const
{
    return mPVWMatrixConstant;
}

inline std::shared_ptr<ConstantBuffer> const& CubeMapEffect::GetWMatrixConstant() const
{
    return mWMatrixConstant;
}

inline std::shared_ptr<ConstantBuffer> const& CubeMapEffect::GetCameraWorldPositionConstant() const
{
    return mCameraWorldPositionConstant;
}

inline std::shared_ptr<ConstantBuffer> const& CubeMapEffect::GetReflectivityConstant() const
{
    return mReflectivityConstant;
}

}
