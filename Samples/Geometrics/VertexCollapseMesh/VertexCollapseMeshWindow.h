// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2018
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.0.0 (2016/06/19)

#pragma once

#include <GTEngine.h>
using namespace gte;

class VertexCollapseMeshWindow : public Window3
{
public:
    VertexCollapseMeshWindow(Parameters& parameters);

    virtual void OnIdle();
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    Vector4<float> mTextColor;
    std::vector<Vector3<float>> mPositions;
    std::vector<std::array<int, 3>> mTriangles;
    std::shared_ptr<VertexCollapseMesh<float>> mVCMesh;
    std::shared_ptr<Visual> mSurface;
    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
};
