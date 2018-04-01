// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.3.0 (2016/09/10)

#pragma once

#include <GTEngine.h>
using namespace gte;

class NURBSCurveExampleWindow : public Window2
{
public:
    NURBSCurveExampleWindow(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    void DoSimulation1();
    void DoSimulation2();
    void InitialConfiguration();
    void NextConfiguration();

    inline void Get(Vector2<float> const& position, int& x, int& y);

    std::shared_ptr<NURBSCurve<2, float>> mSpline;
    std::shared_ptr<NURBSCurve<2, float>> mCircle;
    std::vector<Vector2<float>> mControls;
    std::vector<Vector2<float>> mTargets;
    int mSize;
    float mH, mD;
    float mSimTime, mSimDelta;
    bool mDrawControlPoints;
};

inline void NURBSCurveExampleWindow::Get(Vector2<float> const& position, int& x, int& y)
{
    x = static_cast<int>(position[0] + 0.5f);
    y = mSize - 1 - static_cast<int>(position[1] + 0.5f);
}
