// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/05/02)

#pragma once

#include <Applications/GteWindow3.h>
#include <LowLevel/GteTimer.h>
#include <Graphics/GteDirectionalLightEffect.h>
#include <Physics/GteBoxManager.h>
#include <random>
using namespace gte;

class IntersectingBoxesWindow : public Window3
{
public:
    IntersectingBoxesWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void ModifyBoxes();
    void ModifyMesh(int i);
    void PhysicsTick();
    void GraphicsTick();

    std::vector<AlignedBox3<float>> mBoxes;
    std::unique_ptr<BoxManager<float>> mManager;
    bool mDoSimulation;
    Timer mSimulationTimer;
    double mLastIdle;
    float mSize;

    enum { NUM_BOXES = 16 };

    struct Vertex
    {
        Vector3<float> position, normal;
    };

    std::shared_ptr<Node> mScene;
    std::shared_ptr<RasterizerState> mWireState;
    std::mt19937 mMTE;
    std::uniform_real_distribution<float> mPerturb;
    std::array<std::shared_ptr<Visual>, NUM_BOXES> mBoxMesh;
    std::array<std::shared_ptr<DirectionalLightEffect>, NUM_BOXES> mNoIntersectEffect;
    std::array<std::shared_ptr<DirectionalLightEffect>, NUM_BOXES> mIntersectEffect;
};
