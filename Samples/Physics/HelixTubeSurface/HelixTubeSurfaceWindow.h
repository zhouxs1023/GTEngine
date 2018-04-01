// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.3.0 (2016/07/12)

#pragma once

#include <GTEngine.h>
using namespace gte;

class HelixTubeSurfaceWindow : public Window3
{
public:
    HelixTubeSurfaceWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnKeyDown(int key, int x, int y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateCurve();
    void MoveCamera(float time);

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<NaturalSplineCurve<3, float>> mMedial;
    std::shared_ptr<Visual> mHelixTube;
    float mMinCurveTime, mMaxCurveTime, mCurvePeriod;
    float mCurveTime, mDeltaTime;
};
