// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.3.0 (2016/07/16)

#pragma once

#include <GTEngine.h>
using namespace gte;

#define WATER_DROP_FORMATION_SINGLE_STEP

class WaterDropFormationWindow : public Window3
{
public:
    WaterDropFormationWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateCeilingAndWall();
    void CreateWaterRoot();
    void CreateConfiguration0();  // water surface
    void CreateConfiguration1(); // split into water surface and water drop

    void DoPhysical1();
    void DoPhysical2();
    void DoPhysical3();
    void PhysicsTick();
    void GraphicsTick();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // The scene graph.
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<BlendState> mBlendState;
    VertexFormat mVFormat;
    std::shared_ptr<Node> mScene, mWaterRoot;
    std::shared_ptr<Visual> mCeiling, mWall, mWaterSurface, mWaterDrop;
    std::shared_ptr<Texture2> mWaterTexture;
    std::unique_ptr<RevolutionMesh<float>> mWaterSurfaceMesh;
    std::unique_ptr<RevolutionMesh<float>> mWaterDropMesh;

    // Water curves and simulation parameters.
    std::shared_ptr<NURBSCurve<2, float>> mSpline, mCircle;
    std::vector<Vector2<float>> mTargets;
    float mSimTime, mSimDelta, mLastSeconds;

    // Support for clamping the frame rate.
    Timer mMotionTimer;
    double mLastUpdateTime;
};
