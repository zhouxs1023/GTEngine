// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2018/09/07)

#include "CubeMapEffect.h"
#include <Mathematics/GteVector2.h>
#include <Graphics/GteTexture2Effect.h>

#if defined(GTE_DEV_OPENGL)
#include <Graphics/GL4/GteGL4Engine.h>
#else
#include <Graphics/DX11/GteDX11Engine.h>
#endif

using namespace gte;

CubeMapEffect::CubeMapEffect(std::shared_ptr<ProgramFactory> const& factory,
    Environment const& environment, std::shared_ptr<TextureCube> const& texture,
    float reflectivity, bool& created)
    :
    mCubeTexture(texture),
    mDynamicUpdates(false)
{
    created = false;

    // Load and compile the shaders.
#if defined(GTE_DEV_OPENGL)
    std::string pathVS = environment.GetPath("CubeMapVS.glsl");
    std::string pathPS = environment.GetPath("CubeMapPS.glsl");
    mProgram = factory->CreateFromFiles(pathVS, pathPS, "");
#else
    std::string path = environment.GetPath("CubeMap.hlsl");
    mProgram = factory->CreateFromFiles(path, path, "");
#endif
    if (!mProgram)
    {
        // The program factory will generate Log* messages.
        return;
    }

    // Create the shader constants.
    mWMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);
    SetWMatrix(Matrix4x4<float>::Identity());

    mCameraWorldPositionConstant = std::make_shared<ConstantBuffer>(sizeof(Vector4<float>), true);
    SetCameraWorldPosition(Vector4<float>::Unit(3));

    mReflectivityConstant = std::make_shared<ConstantBuffer>(sizeof(float), true);
    SetReflectivity(reflectivity);

    // Sample using bilinear filtering.
    mCubeSampler = std::make_shared<SamplerState>();
    mCubeSampler->filter = SamplerState::MIN_L_MAG_L_MIP_L;
    mCubeSampler->mode[0] = SamplerState::WRAP;
    mCubeSampler->mode[1] = SamplerState::WRAP;

    // Set the resources for the shaders.
    std::shared_ptr<VertexShader> vshader = mProgram->GetVShader();
    std::shared_ptr<PixelShader> pshader = mProgram->GetPShader();
    vshader->Set("PVWMatrix", mPVWMatrixConstant);
    vshader->Set("WMatrix", mWMatrixConstant);
    vshader->Set("CameraWorldPosition", mCameraWorldPositionConstant);
    pshader->Set("Reflectivity", mReflectivityConstant);
#if defined(GTE_DEV_OPENGL)
    pshader->Set("cubeSampler", mCubeTexture);
#else
    pshader->Set("cubeTexture", mCubeTexture);
#endif
    pshader->Set("cubeSampler", mCubeSampler);

    created = true;
}

void CubeMapEffect::UseDynamicUpdates(float dmin, float dmax)
{
    // Create the camera used to draw each of the 6 faces of the cube.
#if defined(GTE_DEV_OPENGL)
    mCamera = std::make_shared<Camera>(true, false);
#else
    mCamera = std::make_shared<Camera>(true, true);
#endif
    mCamera->SetFrustum(90.0f, 1.0f, dmin, dmax);

    // Create a draw target for the faces.
    mTarget = std::make_shared<DrawTarget>(1, mCubeTexture->GetFormat(),
        mCubeTexture->GetLength(), mCubeTexture->GetLength(), true);
    mTarget->AutogenerateRTMipmaps();
    mTarget->GetRTTexture(0)->SetCopyType(Resource::COPY_STAGING_TO_CPU);

    mDynamicUpdates = true;
}

void CubeMapEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

void CubeMapEffect::UpdateFaces(std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<Spatial> const& scene, Culler& culler,
    Vector4<float> const& envOrigin, Vector4<float> const& envDVector,
    Vector4<float> const& envUVector, Vector4<float> const& envRVector)
{
    Vector4<float> dVector[6] =
    {
        -envRVector,
        envRVector,
        envUVector,
        -envUVector,
        envDVector,
        -envDVector
    };

    Vector4<float> uVector[6] =
    {
        envUVector,
        envUVector,
        -envDVector,
        envDVector,
        envUVector,
        envUVector
    };

    Vector4<float> rVector[6] =
    {
        envDVector,
        -envDVector,
        envRVector,
        envRVector,
        envRVector,
        -envRVector
    };

    // The camera is oriented six times along the coordinate axes and using
    // a frustum with a 90-degree field of view and an aspect ratio of 1 (the
    // cube faces are squares).
    for (int face = 0; face < 6; ++face)
    {
        mCamera->SetFrame(envOrigin, dVector[face], uVector[face], rVector[face]);
        culler.ComputeVisibleSet(mCamera, scene);

        // We need to update the constant buffers that store pvwMatrices.
        // TODO: For the sample application, we know the effects are Texture2Effect.
        // Generally, do we need the ability to query a VisualEffect object for any
        // constant buffers that store pvwMatrices?
        Matrix4x4<float> pvMatrix = mCamera->GetProjectionViewMatrix();
        for (auto visual : culler.GetVisibleSet())
        {
            Texture2Effect* effect = dynamic_cast<Texture2Effect*>(visual->GetEffect().get());
            if (effect)
            {
                // Compute the new projection-view-world matrix.  The matrix
                // *element.first is the model-to-world matrix for the associated
                // object.
#if defined(GTE_USE_MAT_VEC)
                Matrix4x4<float> pvwMatrix = pvMatrix * visual->worldTransform;
#else
                Matrix4x4<float> pvwMatrix = visual->worldTransform * pvMatrix;
#endif
                effect->SetPVWMatrix(pvwMatrix);
                engine->Update(effect->GetPVWMatrixConstant());
            }
        }

        // Draw the scene from the center of the cube to the specified face.
        // The resulting image is stored in the draw target texture.
        engine->Enable(mTarget);
        engine->ClearBuffers();
        for (auto visual : culler.GetVisibleSet())
        {
            engine->Draw(visual);
        }
        engine->Disable(mTarget);

        // Copy the draw target texture to the cube map.  TODO: Implement
        // cube-map render targets to avoid the expensive copies of render
        // target textures to the cube texture.
        auto texture = mTarget->GetRTTexture(0);
        engine->CopyGpuToCpu(texture);

#if !defined(GTE_DEV_OPENGL)
        // The face textures need to be reflected in the u-coordinate to
        // be consistent with the input "*Face.png" cube-map images of
        // this sample.
        unsigned int const numLevels = texture->GetNumLevels();
        for (unsigned int level = 0; level < numLevels; ++level)
        {
            unsigned int uSize = texture->GetDimensionFor(level, 0);
            unsigned int vSize = texture->GetDimensionFor(level, 1);
            unsigned int const* input =
                reinterpret_cast<unsigned int*>(texture->GetDataFor(level));
            unsigned int* output =
                reinterpret_cast<unsigned int*>(mCubeTexture->GetDataFor(face, level));
            for (unsigned int u = 0, uReflect = uSize - 1; u < uSize; ++u, --uReflect)
            {
                for (unsigned int v = 0; v < vSize; ++v)
                {
                    unsigned int src = u + uSize * v;
                    unsigned int trg = uReflect + uSize * v;
                    output[trg] = input[src];
                }
            }
        }
#endif
    }

    engine->CopyCpuToGpu(mCubeTexture);
}
