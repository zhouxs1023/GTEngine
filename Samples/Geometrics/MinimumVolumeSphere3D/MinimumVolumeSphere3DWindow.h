// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.1 (2019/04/13)

#pragma once

#include <Applications/GteWindow3.h>
#include <Mathematics/GteArbitraryPrecision.h>
#include <Mathematics/GteMinimumVolumeSphere3.h>
using namespace gte;

class MinimumVolumeSphere3DWindow : public Window3
{
public:
    MinimumVolumeSphere3DWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void UpdateScene();

    enum { NUM_POINTS = 256 };
    std::array<std::shared_ptr<Visual>, NUM_POINTS> mPoints;
    std::shared_ptr<Visual> mSegments, mSphere;
    std::shared_ptr<RasterizerState> mNoCullWireState;

    int mNumActive;
    std::vector<Vector3<float>> mVertices;
    Sphere3<float> mMinimalSphere;
    MinimumVolumeSphere3<float, BSRational<UIntegerAP32>> mMVS3;
};
