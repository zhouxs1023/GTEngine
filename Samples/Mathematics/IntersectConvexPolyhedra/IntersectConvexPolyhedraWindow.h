// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2019
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 3.24.1 (2019/05/02)

#pragma once

#include <Applications/GteWindow3.h>
#include "ConvexPolyhedron.h"
using namespace gte;

// NOTE: This application is a direct port of the one from Wild Magic 5.
// Most likely the MT* classes can be replaced by the current GTEngine
// support for manifold meshes.  The ConvexPolyhedron code from WM5
// and the ConvexPolyhedron3 code from GTEngine might be reasonable to
// merge into a single class.

class IntersectConvexPolyhedraWindow : public Window3
{
public:
    IntersectConvexPolyhedraWindow(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();
    void ComputeIntersection();

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mMeshPoly0, mMeshPoly1, mMeshIntersection;
    ConvexPolyhedron<float> mPoly0, mPoly1, mIntersection;
    std::array<float, 4> mTextColor;
    std::string mMessage;
};
