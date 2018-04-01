// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2017
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.6.0 (2016/12/09)

#pragma once

#include <GTEngine.h>
#include "ContourFinderEffect.h"
#include "ContourDrawerEffect.h"
using namespace gte;

//#define DRAW_STANDARD_MESH

class ContourEdgesWindow : public Window3
{
public:
    ContourEdgesWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    bool CreateScene();
    bool CreateBackground();
    bool CreateTerrain();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::shared_ptr<Visual> mTerrain;
    std::shared_ptr<Visual> mBackground;
    std::shared_ptr<RasterizerState> mWireState;
    bool mUseWireState;

#if !defined(DRAW_STANDARD_MESH)
    std::shared_ptr<DrawTarget> mFinderTarget;
    float mLineThickness;
    std::shared_ptr<ConstantBuffer> mTerrainID;
    std::shared_ptr<ConstantBuffer> mTerrainPVWMatrix;
    std::shared_ptr<ContourFinderEffect> mTerrainFinderEffect;
    std::shared_ptr<ContourDrawerEffect> mTerrainDrawerEffect;
    std::shared_ptr<ConstantBuffer> mBackgroundID;
    std::shared_ptr<ConstantBuffer> mBackgroundPVWMatrix;
    std::shared_ptr<ContourFinderEffect> mBackgroundFinderEffect;
    std::shared_ptr<ContourDrawerEffect> mBackgroundDrawerEffect;
#endif
};
