// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/04/19)

#pragma once

#include <Applications/GteWindow3.h>
#include <Imagics/GteMarchingCubes.h>
#include <Graphics/GteTexture3Effect.h>
using namespace gte;

//#define USE_DRAW_DIRECT

class SurfaceExtractionWindow : public Window3
{
public:
    SurfaceExtractionWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    enum
    {
        XBOUND = 64,
        YBOUND = 64,
        ZBOUND = 64,
        NUM_VOXELS = XBOUND*YBOUND*ZBOUND,
        NUM_GAUSSIANS = 32,
        XTHREADS = 4,
        YTHREADS = 4,
        ZTHREADS = 4,
        XGROUPS = XBOUND/XTHREADS,
        YGROUPS = YBOUND/YTHREADS,
        ZGROUPS = ZBOUND/ZTHREADS
    };

    bool SetEnvironment();
    bool CreateScene();
    void CreateSharedResources();
    void UpdateConstants();

    std::shared_ptr<RasterizerState> mNoCullSolidState;
    std::shared_ptr<RasterizerState> mNoCullWireState;

    // Resources shared by direct and indirect drawing of voxels.
    MarchingCubes mMarchingCubes;
    std::shared_ptr<StructuredBuffer> mLookup;
    std::shared_ptr<StructuredBuffer> mImage;
    std::shared_ptr<ConstantBuffer> mParametersBuffer;
    float* mLevel;
    Transform mTranslate;
    std::shared_ptr<Texture3> mColorTexture;
    std::shared_ptr<SamplerState> mColorSampler;

#if defined(USE_DRAW_DIRECT)
    // Resources specific to direct drawing.
    void CreateMesh();
    bool CreateDirectResources();

    struct DirectVoxel
    {
        // GLSL will store the first 3 'int' members in a 4-tuple, so the
        // 'unused0' member is padding.
        int configuration;
        int numVertices;
        int numTriangles;
        int unused0;

        Vector4<float> vertices[12];

        // GLSL will store the array in a 16-element chunk of memory, so the
        // 'unused1' member is padding.
        // 'unused0' member is padding.
        int indices[15];
        int unused1;
    };

    struct Vertex
    {
        Vector3<float> position;
        Vector3<float> tcoord;
    };

    std::shared_ptr<StructuredBuffer> mDirectVoxels;
    std::shared_ptr<ComputeProgram> mDirectExtractProgram;
    std::shared_ptr<Texture3Effect> mDirectDrawEffect;
    std::shared_ptr<Visual> mDirectMesh;
#else
    // Resources specific to indirect drawing.
    bool CreateIndirectResources();

    struct IndirectVoxel
    {
        unsigned int index;
        unsigned int configuration;
    };

    std::shared_ptr<Visual> mVoxelMesh;
    std::shared_ptr<StructuredBuffer> mIndirectVoxels;
    std::shared_ptr<ComputeProgram> mIndirectExtractProgram;
    std::shared_ptr<VisualEffect> mIndirectDrawEffect;
    std::shared_ptr<ConstantBuffer> mIndirectPVWMatrixBuffer;
    Matrix4x4<float>* mIndirectPVWMatrix;
#endif
};
